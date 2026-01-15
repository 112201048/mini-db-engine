//Table File Abstraction
//Each TableFile represents a file on disk that stores table data.
#include <fstream>
#include <vector>
using namespace std;

class TableFile {
public:
    TableFile(const string& filename);
    ~TableFile();
    void insertRow(const vector<string>& row);
    vector<vector<string>> readAllRows();
private:
    fstream file;
};