#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

int main()
{
    DIR *dir = opendir("./www/");
    struct dirent *file;
    std::string file_name, extension, sub_folder;

    file = readdir(dir);

    while (file)
    {
        file_name = file->d_name;
        std::cout << file_name << std::endl;
        file = readdir(dir);
    }
    closedir(dir);
    return 0;
}