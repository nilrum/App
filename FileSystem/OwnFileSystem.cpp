//
// Created by user on 23.07.2021.
//

#include "OwnFileSystem.h"
#ifdef FILESYSTEM_OWN

#include <windows.h>
#include "Property/FileFunctions.h"
//#include <shellapi.h>

namespace fs {

    path current_path()
    {
        TCHAR buffer[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, buffer);
        return path(buffer);
    }

    path operator/(const fs::path &lhs, const fs::path &rhs)
    {
        return path(lhs.string() + "/" + rhs.string());
    }

    bool exists(const fs::path &value)
    {
        DWORD dwAttrib = GetFileAttributesW(STR(WStringFromUtf8(value.string())));
        return dwAttrib != INVALID_FILE_ATTRIBUTES;
    }

    bool remove(const fs::path &value)
    {
        auto p = WStringFromUtf8(value.string());
        DWORD dwAttrib = GetFileAttributesW(STR(p));
        if(dwAttrib == INVALID_FILE_ATTRIBUTES) return false;//файл или каталог не найден
        if(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
            return RemoveDirectoryW(STR(p));//удаляем каталог
        else
            return DeleteFileW(STR(p));//удаляем файл
    }

    std::uintmax_t remove_all(const fs::path &value)
    {
        if(fs::exists(value) == false) return false;//каталог уже несуществует
        auto p = WStringFromUtf8(value.string()) + std::wstring (1, L'\0');//value.string() + std::string("\0");//
        SHFILEOPSTRUCTW op;
        memset(&op, sizeof(SHFILEOPSTRUCTW), 0);
        op.wFunc = FO_DELETE;
        op.pFrom = STR(p);
        op.fFlags = FOF_NO_UI;
        return SHFileOperationW(&op) == 0;
    }

    bool create_directory(const fs::path &value)
    {
        return CreateDirectoryW(STR(WStringFromUtf8(value.string())), nullptr);
    }

    bool create_directories(const fs::path &value)
    {
        if(fs::exists(value)) return false;//каталог уже существует
        const auto& p = value.string();
        for(auto it = p.begin(); it != p.end(); it++)
        {
            if((*it == '/' || *it == '\\') && it != p.begin())
            {
                TString dir = TString(p.begin(), it);
                if(fs::exists(dir) == false)
                    create_directory(dir);
            }
        }
        return create_directory(p);
    }

    bool operator==(const path &lhs, const path &rhs)
    {
        return lhs.string() == rhs.string();
    }

    path &path::replace_extension(const path &replacement)
    {
        for(auto it = p.rbegin(); it != p.rend(); it++)
            if(*it == '.')
            {
                p.replace(p.begin() + (p.rend() - it), p.end(), replacement.string());
                break;
            }
            else if(*it == '/' || *it == '\\')
                break;

        return *this;
    }

    path path::parent_path() const
    {
        for(auto it = p.rbegin(); it != p.rend(); it++)
        {
            if (*it == '/' || *it == '\\')
            {
                if ((it + 1) < p.rend())
                    return path(TString(p.begin(), p.begin() + (p.rend() - it - 1)));
                else
                    break;
            }
        }

        return *this;
    }

    path &path::operator/=(const path &value)
    {
        p.append("/");
        p.append(value.string());
        return *this;
    }
}

#endif