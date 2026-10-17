#include <iostream>
#include <utility>
#include <vector>
using namespace std;

/*
Create a HashTable that stores username keys and user records using separate chaining.
Implement insert, find, remove, loadFactor, and automatic resize/rehash.
Use the following educational hash function for the password demonstration. This is for data-structures practice and is not suitable for real password security.
uint64_t demoHash(const std::string& s) {
	uint64_t h = 1469598103934665603ULL;
	for (unsigned char c : s) {
		h = (h ^ c) * 1099511628211ULL;
	}
	return h;
}
Store at least five users. Each user record must include a salt and a hash of salt + password + pepper.
Implement a login flow that looks up the username and verifies the submitted password.
Demonstrate successful login, failed password, unknown user, removal, and a rehash event.
*/

class HTable{
private:
	vector<vector<pair<int, int>>> buckets{10};
	int count = 0;


	int hash(int key){ //hash key
		return key % buckets.size();
	}

	void resish(){ //resize and rehash existing entries
	    vector<vector<pair<int, int>>> newBuckets{buckets.size() * 2};
	    for(int i = 0; i < static_cast<int>(buckets.size()); i++){
	        for(auto& entry : buckets[i]){
	            int newIndex = entry.first % (buckets.size() * 2);
	            newBuckets[newIndex].push_back(entry);
	        }
	    }
	    buckets = std::move(newBuckets);
	}

public:
	void insert(int key, int value){ //inserts a key and a value by into the bucket associated with the hashed key (auto resize + hash)
		for(auto& entry : buckets[hash(key)]){
			if(entry.first == key){
				entry.second = value;
				return;
			}
		}
		buckets[hash(key)].push_back({key, value});
		count++;

		if(loadFactor() > 0.75){
		    resish();
		}
	}

	int find(int key){ //return the value paired with key using key
		for(const auto& entry : buckets[hash(key)]){
			if(entry.first == key){
				return entry.second;
			}
		}
		throw runtime_error("entry dne");
	}

	void remove(int key){ //remove the entry using key
		int index = hash(key);
		for(int i = 0; i < static_cast<int>(buckets[index].size()); i++){
			if(buckets[index][i].first == key){
				buckets[index].erase(buckets[index].begin() + i);
				count--;
				return;
			}
		}
	}

	double loadFactor(){ //return total entries / buckets (average num of entries per bucket)
		return static_cast<double>(count) / buckets.size();
	}
};

int main() {
	HTable table;
	table.insert(10, 10);
	cout << table.find(10) << endl;
	table.insert(20, 20);
	table.insert(10, 30);
	cout << table.find(10) << endl;
	cout << table.find(20) << endl;

	cout << table.loadFactor();


}





















