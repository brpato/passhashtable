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
	int strat = 1; //separate chaining by default because i dont believe in the rest
	vector<vector<pair<int, int>>> buckets{10};
	vector<pair<int, int>> bins{11};
	vector<int> state = vector<int>(11, -1); //-1: untouched, 0: removed, 1: occupied
	int count = 0;

	int hash(const int key) const{ //hash key
		if(strat == 1){return key % buckets.size();}
		else{return key % bins.size();}
	}

	int hash2(const int key) const{ //alternatively hash key
		return 1 + (key % (bins.size() - 1));
	}

	static int nextPrime(int n){
		while(true){
			bool prime = n >= 2;
			for(int i = 2; i <= n / i; i++){
				if(n % i == 0){
					prime = false;
					break;
				}
			}
			if(prime) return n;
			n++;
		}
	}

	void resish(){
		//resize and rehash existing entries
		if(strat == 1){
			vector<vector<pair<int, int>>> newBuckets{buckets.size() * 2};
			for(auto & bucket : buckets){
				for(auto& entry : bucket){
					const int newIndex = entry.first % (newBuckets.size());
					newBuckets[newIndex].push_back(entry);
				}
			}
			buckets = std::move(newBuckets);
		}
		else{ //linear & quadratic probing & double hashing
			vector<pair<int, int>> newBins(nextPrime(bins.size() * 2));
			vector<int> newState(newBins.size(), -1);
			for(int i = 0; i < bins.size(); i++){
				if(state[i]== 1){
					int start = bins[i].first % newBins.size();
					int newIndex = 0;
					bool inserted = false;
					for(int j = 0; j < newBins.size(); j++){
						if(strat == 2){
							newIndex = (start + j) % newBins.size();
						}
						else if(strat == 3){
							newIndex = (start + j * j) % newBins.size();
						}
						else if(strat == 4){
							const int jump = 1 + (bins[i].first % (newBins.size() - 1));
							newIndex = (start + j * jump) % newBins.size();
						}
						if(newState[newIndex] == -1){
							newBins[newIndex] = bins[i];
							newState[newIndex] = 1;
							inserted = true;
							break;
						}
					}
					if(!inserted){
						throw runtime_error("rehash fail");
					}
				}
			}
			bins = std::move(newBins);
			state = std::move(newState);
		}
	}

public:
	void insert(int key, int value){ //inserts a key and a value by into the bucket associated with the hashed key (auto resize + hash)
		double tolerance = 0.75;
		if(strat == 1){ //separate chaining
			for(auto& entry : buckets[hash(key)]){
				if(entry.first == key){
					entry.second = value;
					return;
				}
			}
			buckets[hash(key)].push_back({key, value});
			count++;
		}
		else{ //linear & quadratic probing & double hashing
			tolerance = 0.5;
			int index = 0;
			int firstDeletedIndex = -1;
			for(int i = 0; i < bins.size(); i++){
				if(strat == 2){
					index = (hash(key) + i) % bins.size();
				}
				else if(strat == 3){
					index = (hash(key) + i * i) % bins.size();
				}
				else if(strat == 4){
					index = (hash(key) + i * hash2(key)) % bins.size();
				}
				if(state[index] == 1 && bins[index].first == key){
					bins[index].second = value;
					return;
				}
				if(state[index] == -1){
					break;
				}
				if(state[index] == 0 && firstDeletedIndex == -1){
					firstDeletedIndex = index;
				}
			}
			if(firstDeletedIndex != -1){
				index = firstDeletedIndex;
			}
			if(firstDeletedIndex != -1 || state[index] == -1){
				bins[index] = {key, value};
				state[index] = 1;
				count++;
				return;
			}
		}

		if(loadFactor() >= tolerance){
		    resish();
		}
	}

	int find(const int key) const{ //return the value paired with key using key
		int index = hash(key);
		if(strat == 1){ //separate chaining
			for(const auto& entry : buckets[index]){
				if(entry.first == key){
					return entry.second;
				}
			}
		}
		else{ //linear & quadratic probing & double hashing
			for(int i = 0; i < bins.size(); i++){
				if(strat == 2){
					index = (hash(key) + i) % bins.size();
				}
				else if(strat == 3){
					index = (hash(key) + i * i) % bins.size();
				}
				else if(strat == 4){
					index = (hash(key) + i * hash2(key)) % bins.size();
				}
				if(state[index] == 1 && bins[index].first == key){
					return bins[index].second;
				}
				if(state[index] == -1){
					break;
				}
			}
		}
		throw runtime_error("entry dne");
	}

	void remove(const int key){ //remove the entry using key
		int index = hash(key);
		if(strat == 1){ //separate chaining
			for(int i = 0; i < static_cast<int>(buckets[index].size()); i++){
				if(buckets[index][i].first == key){
					buckets[index].erase(buckets[index].begin() + i);
					count--;
					return;
				}
			}
			cout << "entry dne";
		}
		else{ //linear & quadratic probing & double hashing
			for(int i = 0; i < bins.size(); i++){
				if(strat == 2){
					index = (hash(key) + i) % bins.size();
				}
				else if(strat == 3){
					index = (hash(key) + i * i) % bins.size();
				}
				else if(strat == 4){
					index = (hash(key) + i * hash2(key)) % bins.size();
				}
				if(state[index] == -1){
					break;
				}
				if(state[index] == 1 && bins[index].first == key){
					state[index] = 0;
					count--;
					return;
				}
			}
			cout << "entry dne";
		}
	}

	double loadFactor() const{
		//return total entries / buckets (average num of entries per bucket)
		if(strat == 1) return static_cast<double>(count) / buckets.size();
		return static_cast<double>(count) / bins.size();
	}

	void changeStrat(const int n){ //1: separate chain, 2: linear probe, 3: quadratic probe, 4: double hash
		if(n >= 1 && n <= 4){
			strat = n;
		}
	}

	void print() const{
		if(strat == 1){
			for(int i = 0; i < static_cast<int>(buckets.size()); i++){
				bool first = true;
				cout << "bucket " << i << ": ";
				for(const auto& j : buckets[i]){
					if(!first) cout << ", ";
					cout << "(" << j.first << "|" << j.second << ")";
					first = false;
				}
				cout << endl;
			}
		}
		else{

		}
	}
};

int main(){
	HTable table;
	table.changeStrat(1);
	for(int i = 0; i < 20; i++){
		table.insert(i * 10 + 1, i);
	}

	table.print();
}