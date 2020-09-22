#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace svrlib
{
    class directory
    {
        template<typename THandler>
        static void traverse_folder_imp(const fs::path& path, int depth, THandler&&handler)
        {
            if (depth < 0)
            {
                return;
            }

            if (!fs::exists(path))
            {
                return;
            }

            depth--;

            for (auto&p : fs::directory_iterator(path))
            {
                if (!handler(p.path(), fs::is_directory(p)))
                {
                    break;
                }
                if (fs::is_directory(p))
                {
                    traverse_folder_imp(p.path(), depth, std::forward<THandler>(handler));
                }
            }
        }
    public:
        static std::string current_directory()
        {
            std::error_code ec;
            auto p = fs::current_path(ec);
            return p.string();
        }

        static bool exists(const std::string &path)
        {
            std::error_code ec;
            auto b = fs::exists(path, ec);
            return (!ec) && b;
        }

        //THandler bool(const fs::path& path,bool dir)
        template<typename THandler>
        static void traverse_folder(const std::string& dir, int depth, THandler&&handler)
        {
            traverse_folder_imp(fs::absolute(dir), depth, std::forward<THandler>(handler));
        }

        static bool create_directory(const std::string& dir)
        {
            std::error_code ec;
            fs::create_directories(dir, ec);
            return !ec;
        }

        static bool remove(const std::string& dir)
        {
            std::error_code ec;
            fs::remove(dir, ec);
            return !ec;
        }

        static bool remove_all(const std::string& dir)
        {
            std::error_code ec;
            fs::remove_all(dir, ec);
            return !ec;
        }

        static std::string find_file(const std::string& path, const std::string& filename, int depth = 10)
        {
            std::string result;
            traverse_folder(path, depth, [&result, &filename](const fs::path& p, bool isdir)
            {
                if (!isdir)
                {
                    if (p.filename().string() == filename)
                    {
                        result = fs::absolute(p).string();
                        return false;
                    }
                }
                return true;
            });
            return result;
        }
    };
}