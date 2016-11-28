typedef std::vector<uint8_t> Buffer;

class PTreeTcpClient
{
public:
    PTreeTcpClient(std::string host, int port);
private:
    void handleIncoming();
    std::map<protocol::Uuid, std::string> uuidPathMap;
    std::map<std::string, protocol::Uuid> pathUuidMap;
    std::map<std::string, Buffer> pathValueMap;
    std::mutex uuidPathMapMutex;
    std::mutex pathValueMapMutex;
};