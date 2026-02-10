//Client for the DB engine
#include <iostream>
#include "storage/TableFile.h"
#include "index/BPlusTree.h"
#include <vector>
#include <string>

using namespace std;

int main() {
    try {
        TableFile t("students.db");

        // t.insertRow({"1","Alice"});
        // t.insertRow({"2","Bob"});
        t.deleteByKey(1);
        auto row = t.findByKey(1);
        // cout << "Row for key 2: ";
        for (const auto& col : row) {
            cout << col << " ";
        }
        cout << endl;

        // TableFile tableFile("datatest.db");
        // string tableName = "table";
        // BPlusTree index(3, tableName + "_index.db");

        // for (int i = 1; i <= 50; i++) {
        //     RID r = tableFile.insertRow({to_string(i)});
        //     index.insert(i, r);
        // }

        // for (int i = 1; i <= 50; i++) {
        //     RID out;
        //     if (!index.search(i, out)) {
        //         cout << "Missing key " << i << endl;
        //         return 1;
        //     }
        // }

        // cout << "OK\n";


        // Insert some rows
        // RID r1 = tableFile.insertRow({"Alice", "30", "Engineer"});
        // RID r2 = tableFile.insertRow({"Bob", "25", "Designer"});
        // RID r3 = tableFile.insertRow({"Charlie", "35", "Manager"});
        // RID r4 = tableFile.insertRow({"Diana", "28", "Analyst"});
        
        // BPlusTree index(3); // B+ tree of order 4
        // index.insert(30, r1);
        // index.insert(25, r2);
        // index.insert(35, r3);
        // index.insert(28, r4);

        // RID out;
        // if (index.search(28, out)) {
        //     vector<string> row = tableFile.getRow(out);
        //     cout << "Found row for key 28: ";
        //     for (const auto& col : row) {
        //         cout << col << " ";
        //     }
        //     cout << endl;
        // } else {
        //     cout << "Key 28 not found." << endl;
        // }
        // // Read all rows
        // vector<vector<string>> rows = tableFile.scanAll();
        // for (const auto& row : rows) {
        //     for (const auto& col : row) {
        //         cout << col << " ";
        //     }
        //     cout << endl;
        // }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}