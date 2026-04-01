#include <string>
#include <vector>

class File {
public:
    explicit File(std::string fp) : filePath_{fp} {}

    std::vector<std::string> getAll();
    std::string nextLine();

private:
    size_t lastLineRead_{};
    std::string filePath_{""};
};
