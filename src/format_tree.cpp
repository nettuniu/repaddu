#include "repaddu/format_tree.h"

#include <map>
#include <sstream>

namespace repaddu::format
    {
    namespace
        {
        struct Node
            {
            bool isFile = false;
            std::map<std::string, Node> children;
            };

        void insertPath(Node& root, const std::filesystem::path& path, bool isFile)
            {
            Node* current = &root;
            for (const auto& component : path)
                {
                const std::string name = component.string();
                current = &current->children[name];
                }
            current->isFile = isFile;
            }

        void renderNode(const Node& node, const std::string& prefix, std::ostringstream& out)
            {
            for (auto it = node.children.begin(); it != node.children.end(); ++it)
                {
                const bool isLast = std::next(it) == node.children.end();
                const std::string connector = isLast ? "`-- " : "|-- ";
                out << prefix << connector << it->first;
                if (!it->second.isFile)
                    {
                    out << "/";
                    }
                out << "\n";

                const std::string nextPrefix = prefix + (isLast ? "    " : "|   ");
                renderNode(it->second, nextPrefix, out);
                }
            }
        }

    std::string renderTree(const std::vector<std::filesystem::path>& directories,
        const std::vector<std::filesystem::path>& files)
        {
        Node root;
        for (const auto& dir : directories)
            {
            insertPath(root, dir, false);
            }
        for (const auto& file : files)
            {
            insertPath(root, file, true);
            }

        std::ostringstream out;
        out << ".\n";
        renderNode(root, "", out);
        return out.str();
        }
    }
