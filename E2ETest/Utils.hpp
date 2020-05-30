#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>

inline void clean(propertytree::Client& pClient)
{
    using namespace propertytree;
    auto root = pClient.root();
    root.loadChildren(true);
    struct TraversalContext
    {
        std::vector<std::pair<std::string, Property>> children;
        size_t index = 0;
    };

    std::vector<TraversalContext> levels;

    levels.emplace_back(TraversalContext{root.children(), 0});

    while(true)
    {
        auto* curentLevel = &levels.back();
        size_t* index = &curentLevel->index;

        if (*index >= curentLevel->children.size())
        {
            levels.pop_back();
            if (0 == levels.size())
            {
                break;
            }
            curentLevel = &levels.back();
            index = &curentLevel->index;
        }

        auto& child = curentLevel->children[*index].second;
        if (child.childrenSize())
        {
            levels.emplace_back(TraversalContext{child.children(), 0});
            continue;
        }
        else
        {
            child.destroy();
        }
        (*index)++;
    }
}

#endif // __UTILS_HPP__