#ifndef BYTECODE
#define BYTECODE

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <array>
#include <unordered_map>

typedef std::vector<std::string> string_vector;
typedef std::vector<uint8_t> byte_vector;

enum VariableType: uint8_t {
    CORE, /* like numbers and strings */
    FUNCTION,
    TABLE
};

struct VariableInfo {
    uint type;
    uint id;
};

/* array start */

    /* Initially always false */
    template <typename T>
    struct is_std_array : std::false_type {};

    /* True if the type provided is an array;
    matches std::array<T, N> */
    template <typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};

    /* Array checker */
    template <typename T>
    inline constexpr bool is_std_array_v = is_std_array<T>::value;

/* end */

namespace Bytecode {
    byte_vector byte_set;

    /*
        Key: Variable name (*x)
        Value: Variable type (core? function? table?)
    */
    std::unordered_map<std::string, VariableInfo> variables;
    string_vector keys = {
        "NUMBER",

        "STRING", "STRING_END",
        "TABLE", "TABLE_END",

        "ADD", "SUB", "MUL", "DIV",

        "SET",
        "FETCH", "TABLE_FETCH",
    };
    
    std::unordered_map<std::string, uint8_t> conversions;

    /**
     * @brief Takes the keys from 'keys' and assigns them to
     * actual byte values.
     */
    static void populate_conversions() {
        /* Where the bytes assigned start */
        uint8_t count = 1;

        for (auto k : keys) {
            /* Assign; overwrite */
            conversions[k] = count++;
        }
    }

    /**
     * @brief Converts a floating point number into
     * a 4-byte little endian array.
     * @param number Float
     * @note Works on big-endian machines as well,
     * but the output is always  in little-endian
     * format.
     */
    static std::array<uint8_t, 4> endian(float number) {
        /* Endian floats are stored with 4 bytes(32-bit) */
        std::array<uint8_t, 4> bytes;

        /* Copy the float bytes into the byte array */
        std::memcpy(bytes.data(), &number, sizeof(float));

        /* Because nobody can agree on anything, we need to
        check and see if the machine that this program is
        running on uses big-endian */
        union {uint32_t i; char b[4];} b_int = {0x01020304};
        bool big = (b_int.b[0] == 1);
        if (big) {
            /* Reverse the order */
            std::swap(bytes[0], bytes[3]);
            std::swap(bytes[1], bytes[2]);
        }

        return bytes;
    }

    /**
     * @brief Pushes bytes into the
     * byte set.
     * @param args The arguments; takes
     * any quantity
     */
    template <typename... Args>
    static void add_bytes(const Args&... args) {
        /* Deduces the type of the provided value
        and appropriately handles it */
        auto byte_handler = [](const auto &x) {
            /* Type of x */
            using T = std::decay_t<decltype(x)>;

            if constexpr (is_std_array_v<T> || is_vector_v<T>) {
                /* Current argument is a byte array; loop
                through and add each element inside to the
                byte set */
                for (auto b : x) push(byte_set, b);

            } else if constexpr (std::is_same_v<T, uint8_t>) {
                /* Current argument is a byte; add to the
                byte set directly */
                push(byte_set, x);

            } else {
                /* Unknown */
                std::cout << "Attempted to add an unhandled type to the byte set\n";
            }
        };
        /* Runs each provided argument
        through the lambda function above */
        (byte_handler(args), ...);
    }

    static auto register_variable(const std::string &name, uint8_t type = CORE) {
        if (!variables.contains(name)) {
            /* Variable is not registered yet */
            VariableInfo info;
            info.id = variables.size();
            info.type = type;

            variables[name] = info;
        }
        return variables[name];
    }

    void process(string_vector instruction_set){
        populate_conversions();

        for (auto i : instruction_set) std::cout << i << ' ';
        std::cout << '\n';

        for (auto i : instruction_set) {
            /* Goes through each instruction
            in the instruction set */

            if (conversions.contains(i)) {
                add_bytes(conversions[i]);

            } else if (is_numeric(i)) {
                /* Value is a number;
                convert to little endian
                and add to the byte set */
                float number = std::stof(i);
                add_bytes(endian(number));

            } else if (i[0] == '*') {
                /* Variable pointer */

                /* Get variable information */
                auto info = register_variable(i);

                /* Push ID */
                float index = (float)info.id;
                add_bytes(endian(index));

            } else if (i[0] == '&') {
                /* Table pointer */
                i.erase(0, 1); /* Registered as *x */

                /* Get variable information */
                auto info = register_variable("*" + i, TABLE);

                /* Push ID */
                float index = (float)info.id;
                add_bytes(endian(index));

            } else if (i[0] == '"') {
                /* String */
                i.erase(0, 1);
                byte_vector bytes(i.begin(), i.end());

                add_bytes(bytes);

            } else {
                std::cout << "Unhandled instruction |> " << i << '\n';
            }
        }
        for (auto b : byte_set) {
            std::printf("%02X ", b);
        }
    }
}

#endif