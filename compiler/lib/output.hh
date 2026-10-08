#ifndef OUTPUT
#define OUTPUT

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace Output {
    /* vector start */

        /* Initially always false */
        template <typename T>
        struct is_vector : std::false_type {};

        /* Returns true if the provided type matches
        std::vector<T, Alloc> */
        template <typename T, typename Alloc>
        struct is_vector<std::vector<T, Alloc>> : std::true_type {};

        /* Vector checker */
        template <typename T>
        inline constexpr bool is_vector_v = is_vector<T>::value;

    /* end */

    #define RED "\033[31m"
    #define RST "\033[0m"

    /**
     * @brief Appends small strings onto
     * one main string.
     * @param str Main string
     * @param args Strings to append
     */
    template <typename... Args>
    void string_append(std::string &str, const Args&... args) {
        /* Append each argument, convert to std::string
        to prevent corrupt appendation */
        (str.append(std::string(args) + '\n'), ...);
    }

    /**
     * @brief Displays an error message with the provided
     * content and terminates the program.
     */
    void error(const std::string &content) {
        std::cout << "qzc.exe: " << RED << "error: " << RST << content << '\n';
        std::exit(1);
    }
    /**
     * @brief Checks that a provided ofstream file
     * object successfully opened. If not, then the program
     * is terminated.
     * @param file File
     * @param path File path
     */
    static void check_file(std::ofstream &file, const std::string &path = "unknown") {
        if (!file) {
            std::cout << "Could not open file \"" + path + "\"\n";
            std::exit(1);
        }
    }

    /**
     * @brief Writes content to a specified file
     * path.
     * @param path File path
     * @param content Content
     * @note Takes both vectors and strings.
     */
    template <typename T>
    void write(const std::string &path, T content) {
        std::ofstream file;

        if constexpr (is_vector_v<T>) {
            /* Provided content is in
            the form of a vector */

            file.open(path, std::ios::binary);
            check_file(file, path);

            /* Write as const char* */
            file.write(reinterpret_cast<const char*>(content.data()), content.size());

        } else if constexpr (
               std::is_same_v<T, std::string>
            || std::is_same_v<T, const char*>
            || std::is_convertible_v<T, std::string>
        ) {
            /* Provided content is in
            the form of a string */

            file.open(path);
            check_file(file, path);

            file << content;
        }

        file.close();
    }
}

#endif