#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <list>
#include <sstream>
#include <algorithm>
#include <functional>
#include <utility>

using namespace std;

struct TreeNode {
    int version_id;
    string content;
    string message;
    time_t created_timestamp;
    time_t snapshot_timestamp;
    TreeNode* parent;
    vector<TreeNode*> children;

    TreeNode (int id, string c="", string m="")
    {
        version_id = id;
        content = c;
        message = m;
        parent = nullptr;
        created_timestamp = time(NULL);
        snapshot_timestamp = 0;
    }
};

class File;

class HashMapInt {
    private:
    vector<list<pair<int, TreeNode*>>> table;
    int size;

    public:
    HashMapInt(int n=1000) {
        size = n;
        table.resize(size);
    }
    int hashFunction(int key) {
        return key%size;
    }
    void put(int key, TreeNode* val) {
        int index = hashFunction(key);
        for(auto &p : table[index]) {
            if(p.first == key) {
                p.second = val;
                return;
            }
        }
        table[index].push_back({key,val});
    }
    TreeNode* get(int key) {
        int index = hashFunction(key);
        for(auto &p : table[index]) {
            if(p.first == key) return p.second;
        }
        return nullptr;
    }
};

class HashMapString {
    private:
    vector<list<pair<string, File*>>> table;
    int size;

    unsigned long long polyHash(const string &s) {
        const unsigned long long p = 31;
        const unsigned long long M = 1000000009ULL;
        unsigned long long h = 0;
        unsigned long long power = 1;

        for(char c : s) {
            h = (h+ (unsigned long long)(c) * power) % M;
            power = (power * p) % M;
        }
        return h;
    }

    public:
    HashMapString(int n=1000) {
        size = n;
        table.resize(size);
    }
    int hashFunction(const string &s) {
        unsigned long long raw = polyHash(s);
        return (int)(raw%size);
    }
    void put(string key, File* val) {
        int index = hashFunction(key);
        for(auto &p : table[index]) {
            if(p.first == key) {
                p.second = val;
                return;
            }
        }
        table[index].push_back({key,val});
    }

    File* get(string key) {
        int index = hashFunction(key);
        for(auto &p : table[index]) {
            if(p.first == key) return p.second;
        }
        return nullptr;
    }
};

class File {
    private:
    TreeNode* root;
    TreeNode* active_version;
    HashMapInt version_map;
    int total_versions;
    string name;
    time_t last_modified;

    public:
    File(string file_name) : version_map(1000) {
        name = file_name;
        root = new TreeNode(0,"", "Initial Snapshot");
        root->snapshot_timestamp = time(NULL);
        active_version = root;
        version_map.put(0,root);
        total_versions = 1;
        last_modified = time(NULL);
    }
    ~File() {
        freeTree(root);
    }
    void freeTree(TreeNode* node) {
        if(!node) return;
        for(TreeNode* child : node->children) {
            freeTree(child);
        }
        delete node;
    }
    void read() {
        cout << active_version->content << endl;
    }

    void insert(string c) {
        if(active_version->snapshot_timestamp != 0) {
            TreeNode* node = new TreeNode(total_versions, active_version->content + c);
            node->parent = active_version;
            active_version->children.push_back(node);
            active_version = node;
            version_map.put(node->version_id, node);
            total_versions += 1;
        }
        else {
            active_version->content += c;
        }
        last_modified = time(NULL);
    }

    void update(string c) {
        if(active_version->snapshot_timestamp != 0) {
            TreeNode* node = new TreeNode(total_versions, c);
            node->parent = active_version;
            active_version->children.push_back(node);
            active_version = node;
            version_map.put(node->version_id, node);
            total_versions += 1;
        }
        else {
            active_version->content = c;
        }
        last_modified = time(NULL);
    }

    void snapshot(string m) {
        if(active_version->snapshot_timestamp == 0) {
            active_version->snapshot_timestamp = time(NULL);
            active_version->message = m;
        }
        else {
            TreeNode* node = new TreeNode(total_versions, active_version->content, m);
            node->parent = active_version;
            active_version->children.push_back(node);
            node->snapshot_timestamp = time(NULL);
            active_version = node;
            version_map.put(node->version_id, node);
            total_versions++;
        }
        //snapshot not considered a modification (as stated by Laskshay sir on MS Teams) so last_modified time not updated
    }

    void rollback(int rollback_ver_id = -1) {
        if(rollback_ver_id == -1) {
            if(active_version->parent) {
                active_version = active_version->parent;
            }
            else {
                cout << "ERROR: Rollback failed: already at root, cannot rollback further" << endl;
            }
        }
        else {
            TreeNode* node = version_map.get(rollback_ver_id);
            if(node) {
                active_version = node;
            }
            else {
                cout << "ERROR: Rollback failed: version ID " << rollback_ver_id << " does not exist" << endl;
            }
        }
    }

    void history() {
        TreeNode* current = active_version;
        vector<TreeNode*> snapshots_till_now;
        while(current) {
            if(current->snapshot_timestamp != 0) snapshots_till_now.push_back(current);
            current = current->parent;
        }
        reverse(snapshots_till_now.begin(), snapshots_till_now.end());

        for(auto s : snapshots_till_now) {
            cout << "Version " << s->version_id <<  " | Snapshot: " << string(ctime(&s->snapshot_timestamp)).substr(0,24) << " | Message: " << s->message << endl;
        }
    }
    friend class FileSystem;

};



template<typename T>
class MaxHeap {
    private:
    vector<T> data;
    function<bool(const T&, const T&)> comparator;

    void heapifyUp(int i) {
        while(i>0) {
            int p = (i-1)/2;
            if(comparator(data[i], data[p])) {
                swap(data[i], data[p]);
                i = p;
            }
            else break;
        }
    }
    void heapifyDown(int i) {
        int n = data.size();
        while(true) {
            int l = 2*i+1;
            int r = 2*i+2;
            int big = i;
            if(l<n && comparator(data[l], data[big])) big = l;
            if(r<n && comparator(data[r], data[big])) big = r;
            if(big != i) {
                swap(data[i], data[big]);
                i = big;
            }
            else break;
        }
    }

    public:
    MaxHeap(function<bool(const T&, const T&)> c) : comparator(c) {}
    void push(T val) {
        data.push_back(val);
        heapifyUp(data.size()-1);
    }
    bool empty() {
        return data.empty();
    }
    T top() {
        return data[0];
    }
    void pop() {
        if(data.empty()) return;

        data[0] = data.back();
        data.pop_back();
        if(!data.empty()) heapifyDown(0);
    }
};

class FileSystem {
    private:
    HashMapString file_map;
    vector<File*> all_files;

    MaxHeap<pair<time_t, string>> recentHeap;
    MaxHeap<pair<int,string>> biggestHeap;

    public:
    FileSystem()
        : file_map(1000), recentHeap([](const pair<time_t,string> &a, const pair<time_t, string> &b) {return a.first > b.first;}), biggestHeap([](const pair<int,string> &a, const pair<int,string> &b) {return a.first > b.first;}) {}
    
    ~FileSystem() {
        for(File* f : all_files) {
            delete f;
        }
    }
    void create(string fileName) {
        if(file_map.get(fileName)) {
            cout << "ERROR: File already exists: " << fileName << endl;
            return;
        }
        File* f = new File(fileName);
        file_map.put(fileName, f);
        all_files.push_back(f);
        updateHeaps(f);
    }
    void updateHeaps(File* f) {
        recentHeap.push({f->last_modified, f->name});
        biggestHeap.push({f->total_versions, f->name});
    }
    void process(string cmd) {
        stringstream ss(cmd);
        string op;
        string fileName;
        string arg;
        ss >> op;
        if(op == "CREATE") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing filename for CREATE" << endl;
                return;
            }
            create(fileName);
        }
        else if(op == "READ") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing filename for READ" << endl;
                return;
            }
            File* f = file_map.get(fileName);
            if(f) f->read();
            else cout << "ERROR: File not found: " << fileName << endl;
        }
        else if(op == "INSERT") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing argument(s) for INSERT" << endl;
                return;
            }
            getline(ss,arg);
            if(arg.size() && arg[0] == ' ') arg = arg.substr(1);
            if(arg.empty()) {
                cout << "ERROR: Missing argument(s) for INSERT" << endl;
                return;
            }
            File* f = file_map.get(fileName);
            if(f) {
                f->insert(arg);
                updateHeaps(f);
            }
            else {
                cout << "ERROR: File not found: " << fileName << endl;
            }
        }
        else if(op == "UPDATE") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing argument(s) for UPDATE" << endl;
                return;
            }
            getline(ss, arg);
            if(arg.size() && arg[0] == ' ') arg = arg.substr(1);
            if(arg.empty()) {
                cout << "ERROR: Missing argument(s) for UPDATE" << endl;
                return;
            }
            File* f = file_map.get(fileName);
            if(f) {
                f->update(arg);
                updateHeaps(f);
            }
            else {
                cout << "ERROR: File not found: " << fileName << endl;
            }
        }
        else if(op == "SNAPSHOT") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing argument(s) for SNAPSHOT" << endl;
                return;
            }
            getline(ss, arg);
            if(arg.size() && arg[0] == ' ') arg = arg.substr(1);
            if(arg.empty()) {
                cout << "ERROR: Missing argument(s) for SNAPSHOT" << endl;
                return;
            }
            File* f = file_map.get(fileName);
            if (f) {
                f->snapshot(arg);
            }
            else {
                cout << "ERROR: File not found: " << fileName << endl;
            }
        }
        else if(op == "ROLLBACK") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing argument(s) for ROLLBACK" << endl;
                return;
            }
            File* f = file_map.get(fileName);
            if(!f) {
                cout << "ERROR: File not found: " << fileName << endl;
                return;
            }
            int ver_id;
            if(ss >> ver_id) f->rollback(ver_id);
            else f->rollback();
        }
        else if(op == "HISTORY") {
            if(!(ss >> fileName)) {
                cout << "ERROR: Missing filename for HISTORY" << endl;
                return;
            }
            File* f = file_map.get(fileName);
            if(f) f->history();
            else cout << "ERROR: File not found: " << fileName << endl;
        }
        else if(op == "RECENT_FILES") {
            int num;
            if(!(ss >> num)) {
                cout << "ERROR: Missing number for RECENT_FILES" << endl;
                return;
            }
            // MaxHeap<pair<time_t, string>> tempHeap = recentHeap;
            int printed = 0;
            HashMapString seen(num*2);
            vector<pair<time_t,string>> reinserts;

            while(!recentHeap.empty() && printed < num) {
                auto [t, name] = recentHeap.top();
                recentHeap.pop();
                File* f = file_map.get(name);
                // if(f && f->last_modified == t && seen.get(name) == NULL) {
                //     cout << name << endl;
                //     seen.put(name, f);
                //     printed += 1;
                // }
                if(!f) continue;
                if(f->last_modified != t) continue;
                if(seen.get(name) != NULL) continue;
                cout <<  name << endl;
                seen.put(name,f);
                printed +=1;
                reinserts.push_back({t,name});
            }
            for(auto &entry : reinserts) {
                recentHeap.push(entry);
            }
        }
        else if(op == "BIGGEST_TREES") {
            int num;
            if(!(ss >> num)) {
                cout << "ERROR: Missing number for BIGGEST_TREES" << endl;
                return;
            }

            // MaxHeap<pair<int,string>> tempHeap = biggestHeap;
            int printed = 0;
            HashMapString seen(num*2);
            vector<pair<int,string>> reinserts;
            
            while(!biggestHeap.empty() && printed < num) {
                auto [cnt, name] = biggestHeap.top();
                biggestHeap.pop();
                File* f = file_map.get(name);
                // if(f && f->total_versions == cnt && seen.get(name) == NULL) {
                //     cout << name << " " << cnt <<  endl;
                //     seen.put(name, f);
                //     printed += 1;
                // }
                if(!f) continue;
                if(f->total_versions != cnt) continue;
                if(seen.get(name) != NULL) continue;

                cout << name << " " << cnt << endl;
                seen.put(name,f);
                printed += 1;

                reinserts.push_back({cnt,name});
            }
            for(auto &entry : reinserts) {
                biggestHeap.push(entry);
            }
        }
        else if(op == "HELP") {
            cout << "================ Available Commands ================\n";
            cout << "Core File Operations:\n";
            cout << "  CREATE <filename>\n";
            cout << "     Creates a file with a root version (ID 0), empty content, and an initial snapshot message.\n";
            cout << "     Note: The root is marked as a snapshot.\n\n";

            cout << "  READ <filename>\n";
            cout << "     Displays the content of the file's currently active version.\n\n";

            cout << "  INSERT <filename> <content>\n";
            cout << "     Appends content to the file.\n";
            cout << "     - If active version is a snapshot → creates a new version.\n";
            cout << "     - Otherwise → modifies the active version in place.\n\n";

            cout << "  UPDATE <filename> <content>\n";
            cout << "     Replaces the file's content.\n";
            cout << "     - Same versioning logic as INSERT.\n\n";

            cout << "  SNAPSHOT <filename> <message>\n";
            cout << "     Marks the active version as a snapshot (immutable).\n";
            cout << "     Stores the provided message and current time.\n\n";

            cout << "  ROLLBACK <filename> [versionID]\n";
            cout << "     Sets the active version pointer to the given versionID.\n";
            cout << "     - If no ID provided → rolls back to the parent of the current version.\n\n";

            cout << "  HISTORY <filename>\n";
            cout << "     Lists all snapshotted versions of the file chronologically\n";
            cout << "     (on the path from active node to the root).\n";
            cout << "     Displays ID, timestamp, and message.\n\n";

            cout << "System-Wide Analytics:\n";
            cout << "  RECENT_FILES [num]\n";
            cout << "     Lists files in descending order of last modification time.\n";
            cout << "     Restricted to the first [num] entries.\n\n";

            cout << "  BIGGEST_TREES [num]\n";
            cout << "     Lists files in descending order of their total version count.\n";
            cout << "     Restricted to the first [num] entries.\n\n";

            cout << "===================================================" << endl;
        }

        else {
            cout << "ERROR: Unknown command '" << op << "'" << endl;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    FileSystem fs;
    string line;
    cout << "====================== START ======================" << endl;
    cout << "  HELP\n";
    cout << "     Displays the help message.\n";
    cout << "  EXIT\n";
    cout << "     Exits the program.\n";
    cout << "================= ENTER COMMANDS =================" << endl;
    while (getline(cin,line)) {
        if(line.empty()) continue;
        if(line == "EXIT") {
            cout << "======================= END =======================" << endl;
            break;
        }

        fs.process(line);
    }
    return 0;
}