#include "meow_vm.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [--binary] <entry_file>" << std::endl;
        return 1;
    }

    std::string entryPath;
    bool isBinary = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--binary") {
            isBinary = true;
        } else if (entryPath.empty()) {
            entryPath = arg;
        }
    }

    if (entryPath.empty()) {
        std::cerr << "Lỗi: Bạn phải cung cấp tên file đầu vào." << std::endl;
        return 1;
    }

    MeowVM vm(".", argc, argv);
    

    vm.interpret(entryPath, isBinary);
    
    return 0;
}