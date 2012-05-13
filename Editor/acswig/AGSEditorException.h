#include <stdexcept>

class AGSEditorException : public std::runtime_error {
 public:
   AGSEditorException(const std::string& msg);
};

