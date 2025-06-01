#include <termios.h>

#include <regex>
#include <cctype>

#include <bfc/CommandManager.hpp>

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>
#include <logless/Logger.hpp>

namespace propertytree
{

std::vector<uint8_t> hexStringToVector(const std::string& pHex)
{
    std::vector<uint8_t>  rv;
    bool nibble = false;
    uint8_t byte = 0;
    for (auto i : pHex)
    {
        i = std::tolower(i);
        i = i >= 'a' ? i - 'a' + 10 : i - '0';
        if (!nibble)
        {
            byte = i << 4;
        }
        else
        {
            byte |= i;
            rv.emplace_back(byte);
        }

        nibble = !nibble;
    }
    return rv;
}

class Monitor
{
public:
    Monitor()
        : mClient(ClientConfig{"127.0.0.1", 12345})
    {
        initTerminal();

        mCmdMan.addCommand("exit", [this](bfc::ArgsMap&&) -> std::string {
                mRunning = false;
                mToSubscribeCv.notify_one();
                return "exiting...";
            });

        mToSubscribeWorker = std::thread([this](){
                while(mRunning)
                {
                    std::unique_lock<std::mutex> lgToSubscribe(mToSubscribeMutex);
                    mToSubscribeCv.wait(lgToSubscribe, [this](){
                            return mToSubscribe.size() || !mRunning;
                        });

                    auto tosub = std::move(mToSubscribe);
                    lgToSubscribe.unlock();
                    for (auto& prop : tosub)
                    {
                        prop.subscribe();
                        prop.setUpdateHandler([this, prop]() mutable {
                                auto data = prop.raw();
                                consoleLog("[Monitor]: Property CHG uuid=", prop.uuid(), " name=\"", prop.name(), "\" data=[", data.size(), "]{", toHexString(data.data(), data.size()) ,"}");
                            });
                    }
                }
            });

        mClient.setTreeAddHandler([this](Property pProp){
                auto parent = pProp.parent();
                std::string parentInfo;
                if (parent)
                {
                    parentInfo = " parentName=\"";
                    parentInfo += parent->name();
                    parentInfo += "\" parentUuid=";
                    parentInfo += std::to_string(parent->uuid());
                }

                consoleLog("[Monitor]: Property ADD uuid=", pProp.uuid(), " name=\"", pProp.name(), "\"", parentInfo);

                if (!mAutowatch)
                {
                    return;
                }

                std::unique_lock<std::mutex> lgToSubscribe(mToSubscribeMutex);
                mToSubscribe.emplace_back(pProp);
                mToSubscribeCv.notify_one();
            });

        mClient.setTreeRemoveHandler([this](Property pProp){
                consoleLog("[Monitor]: Property DEL uuid=", pProp.uuid(), " name=\"", pProp.name(), "\"");
            });

        mCmdMan.addCommand("create", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdCreate(std::move(pMap));});
        mCmdMan.addCommand("set", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdSet(std::move(pMap));});
        mCmdMan.addCommand("get", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdGet(std::move(pMap));});
        mCmdMan.addCommand("list", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdList(std::move(pMap));});
        mCmdMan.addCommand("autowatch", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdAutoWatch(std::move(pMap));});
        mCmdMan.addCommand("subscribe", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdSubscribe(std::move(pMap));});
        mCmdMan.addCommand("unsubscribe", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdUnsubscribe(std::move(pMap));});
        mCmdMan.addCommand("delete", [this](bfc::ArgsMap&& pMap) -> std::string {return onCmdDelete(std::move(pMap));});
    }

    void run()
    {
        while(mRunning)
        {
            char key;
            read(STDIN_FILENO, &key, 1);
            consoleIn(key);
        }
    }

private:

    std::string onCmdCreate(bfc::ArgsMap&& pMap)
    {
        auto parent = pMap.arg("parent");
        if (!parent)
        {
            return "parent not specified!";
        }

        auto name = pMap.arg("name");
        if (!name)
        {
            return "name not specified!";
        }

        auto prop = getByPath(*parent);
        if (!prop)
        {
            return "property not found!";
        }

        auto created = prop.create(*name);
        if (created)
        {
            return "done!";
        }
        return "failed!";
    }

    std::string onCmdSet(bfc::ArgsMap&& pMap)
    {
        auto path = pMap.arg("path");
        auto value = pMap.arg("value");
        if (!path)
        {
            return "path not specified!";
        }
        if (!value)
        {
            return "value not specified!";
        }

        auto prop = getByPath(*path);
        if (!prop)
        {
            return "property not found!";
        }

        auto val = hexStringToVector(*value);

        prop.set(std::move(val));

        return "";
    }

    std::string onCmdGet(bfc::ArgsMap&& pMap)
    {
        auto path = pMap.arg("path");
        if (!path)
        {
            return "path not specified!";
        }

        auto prop = getByPath(*path);
        if (!prop)
        {
            return "property not found!";
        }

        prop.fetch();
        auto value = prop.raw();
        return std::string("value=") + toHexString(value.data(), value.size());
    }

    std::string onCmdList(bfc::ArgsMap&& pMap)
    {
        auto root = mClient.root();
        auto path = pMap.arg("path");
        auto act = pMap.arg("action");

        if (path)
        {
            root = getByPath(*path);
        }

        enum {NONE, DEL, SUB, UNSUB} action;

        if (act && "delete" == *act) action = DEL;
        else if (act && "subscribe" == *act) action = SUB;
        else if (act && "unsubscribe" == *act) action = UNSUB;

        root.loadChildren(true);

        struct TraversalContext
        {
            std::string name;
            std::vector<std::pair<std::string, Property>> children;
            size_t index = 0;
        };

        std::vector<TraversalContext> levels;

        levels.emplace_back(TraversalContext{"", root.children(), 0});

        std::stringstream ss;
        ss << "\n|-/\n";
        while(true)
        {
            auto* curentLevel = &levels.back();
            size_t* index = &curentLevel->index;

            if (curentLevel->children.size() > *index)
            {
                auto name = curentLevel->children[*index].first;
                auto& prop = curentLevel->children[*index].second;
                prop.fetch();

                if (SUB == action)
                {
                    prop.subscribe();
                    prop.setUpdateHandler([this, prop]() mutable {
                            auto data = prop.raw();
                            consoleLog("[Monitor]: Property CHG uuid=", prop.uuid(), " name=\"", prop.name(), "\" data=[", data.size(), "]{", toHexString(data.data(), data.size()) ,"}");
                        });
                }
                else if (UNSUB == action)
                {
                    prop.unsubscribe();
                }
                auto data = prop.raw();
                ss << std::string(levels.size()*2, ' ') << "|-/" << name << " uuid=" << prop.uuid() << " data[" << data.size() <<  "]={" << toHexString(data.data(), data.size()) << "}\n";
            }

            if (curentLevel->children.size() <= *index)
            {
                levels.pop_back();
                if (0 == levels.size())
                {
                    break;
                }

                curentLevel = &levels.back();
                index = &curentLevel->index;
                size_t oldIndex = *index-1;
                auto& child = curentLevel->children[oldIndex].second;
                if (!child.childrenSize() && DEL == action)
                {
                    child.destroy();
                }
                continue;
            }

            size_t oldIndex = *index;
            (*index)++;

            auto& child = curentLevel->children[oldIndex].second;
            if (child.childrenSize())
            {
                auto name = curentLevel->children[oldIndex].first;
                levels.emplace_back(TraversalContext{name, child.children(), 0});
            }
            else if (DEL == action)
            {
                child.destroy();
            }
        }

        return std::move(ss.str());
    }

    std::string onCmdSubscribe(bfc::ArgsMap&& pMap)
    {
        auto path = pMap.arg("path");
        if (!path)
        {
            return "path not specified!";
        }

        auto prop = getByPath(*path);
        if (!prop)
        {
            return "property not found!";
        }
        prop.subscribe();
        prop.setUpdateHandler([this, prop]() mutable {
                auto data = prop.raw();
                consoleLog("[Monitor]: Property CHG uuid=", prop.uuid(), " name=\"", prop.name(), "\" data=[", data.size(), "]{", toHexString(data.data(), data.size()) ,"}");
            });
        return "subscribed!";
    }

    std::string onCmdUnsubscribe(bfc::ArgsMap&& pMap)
    {
        auto path = pMap.arg("path");
        if (!path)
        {
            return "path not specified!";
        }

        auto prop = getByPath(*path);
        if (!prop)
        {
            return "property not found!";
        }
        prop.unsubscribe();
        return "unsubscribed!";
    }

    std::string onCmdDelete(bfc::ArgsMap&& pMap)
    {
        auto path = pMap.arg("path");
        if (!path)
        {
            return "path not specified!";
        }

        auto prop = getByPath(*path);
        if (!prop)
        {
            return "property not found!";
        }

        return prop.destroy() ? "deleted!" : "failed!";
    }

    std::string onCmdAutoWatch(bfc::ArgsMap&& pMap)
    {
        auto enabled = pMap.arg("enabled");

        if (enabled && "false" == *enabled)
        {
            mAutowatch = false;
            return "autowatch disabled!";
        }
        mAutowatch = true;
        return "autowatch enabled!";
    }

    Property getByPath(std::string pPath)
    {
        if (pPath == "/")
        {
            return mClient.root();
        }

        const std::regex separator("/([A-Za-z0-9]+)");
        std::smatch match;

        std::sregex_iterator next(pPath.begin(), pPath.end(), separator);
        std::sregex_iterator end;

        Property current = mClient.root();

        while (next != end)
        {
            std::smatch match = *next;
            current = current.get(match[1]);
            if (!current)
            {
                break;
            }
            next++;
        }

        return current;
    }

    void consoleIn(char pKey)
    {
        if (mConsolseWaitChar)
        {
            mConsolseWaitChar--;
            return;
        }

        if (27 == pKey)
        {
            mConsolseWaitChar = 2;
            return;
        }

        if (9 == pKey)
        {
            return;
        }
        if (127 == pKey)
        {
            if (0 == mConsoleInputBufferIdx)
            {
                return;
            }

            mConsoleInputBufferIdx--;
            std::cout << "\b \b" << std::flush;

            return;
        }

        if (mConsoleInputBufferIdx>sizeof(mConsoleInputBuffer)-1)
        {
            return;
        }

        if (10 == pKey)
        {
            std::cout << "\n";
            mConsoleInputBuffer[mConsoleInputBufferIdx] = 0;
            mConsoleInputBufferIdx = 0;
            std::string command = mConsoleInputBuffer;
            auto res = mCmdMan.executeCommand(command);
            if (!res.size())
            {
                printTermInput();
            }
            consoleLog(command, " : ", res);
            return;
        }

        std::cout << pKey << std::flush;
        mConsoleInputBuffer[mConsoleInputBufferIdx++] = pKey;
    }

    void initTerminal()
    {
        termios term = {0};

        if (tcgetattr(0, &term) < 0)
        {
            throw std::runtime_error(strerror(errno));
        }

        term.c_lflag &= ~ICANON;
        term.c_lflag &= ~ECHO;
        term.c_cc[VMIN] = 1;
        term.c_cc[VTIME] = 0;

        if (tcsetattr(0, TCSANOW, &term) < 0)
        {
            throw std::runtime_error(strerror(errno));
        }

        consoleLog();
    }

    template<typename... T>
    void consoleLog(T&&... ts)
    {
        std::stringstream ss;
        consoleLog_(ss, ts...);
        std::cout << "\r\033[K" << ss.str() << "\n";
        printTermInput();
    }

    template<typename... T>
    void consoleLog_(T&&... ts)
    {
        (... << std::forward<T>(ts));
    }

    void consoleLog(const std::string& pStr)
    {
        consoleLog(pStr.c_str());
    }

    void printTermInput()
    {
        std::string_view currentCmd(mConsoleInputBuffer, mConsoleInputBufferIdx);
        std::cout << "$user: " << currentCmd << std::flush;
    }

    propertytree::Client mClient;

    bool mRunning = true;
    char mConsoleInputBuffer[128];
    size_t mConsoleInputBufferIdx = 0;
    int mConsolseWaitChar = 0;

    bool mAutowatch = false;

    bfc::CommandManager mCmdMan;

    std::vector<Property> mToSubscribe;
    std::mutex mToSubscribeMutex;
    std::condition_variable mToSubscribeCv;
    std::thread mToSubscribeWorker;
};

} // namespace propertytree

int main()
{
    // Logger::getInstance().logful();

    propertytree::Monitor m;
    m.run();
    return 0;
}
