//
// Created by user on 23.07.2021.
//

#ifndef ATLAS_OWNFILESYSTEM_H
#define ATLAS_OWNFILESYSTEM_H

#ifdef FILESYSTEM_OWN
#include <string>

namespace fs{
    using TString = std::string;
    struct path{
        path(const TString& value = TString()):p(value){};
        path(const char* value):p(value){};
        //path filename() const;
        const TString& string() const { return p; };
        path& replace_extension( const path& replacement = path() );

        //path root_name() const;       //С: или ""
        //path root_directory() const;    //"\" или "/"
        path parent_path() const;

        path& operator /=(const path& value);
    private:
        TString p;
    };

    bool operator==(const path& lhs, const path& rhs);

    path current_path();
    path operator/( const path& lhs, const path& rhs );

    bool exists(const path& value);
    bool remove(const path& value);
    std::uintmax_t remove_all(const path& value);

    bool create_directory( const path& value);
    bool create_directories( const path& value);
}
#endif

#endif //ATLAS_OWNFILESYSTEM_H
