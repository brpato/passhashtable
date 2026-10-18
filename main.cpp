#include <cstdint>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <random>
#include <functional>
#include <optional>
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

struct Credentials{
	string salt;
	uint64_t passwordHash;
};

const string pepper = "s4lt";

static string genSalt(){
	random_device rd;
	return to_string(rd());
}

uint64_t demoHash(const std::string& s){
	uint64_t h = 1469598103934665603ULL;
	for (unsigned char c : s){
		h = (h ^ c) * 1099511628211ULL;
	}
	return h;
}

Credentials createPass(const string& password){
	string salt = genSalt();
	return {salt, demoHash(salt + password + pepper)};
}



class HTable{
private:
	int strat = 1; //separate chaining by default because i dont believe in the rest
	vector<vector<pair<string, Credentials>>> buckets{10};
	vector<pair<string, Credentials>> bins{11};
	vector<int> state = vector<int>(11, -1); //-1: untouched, 0: removed, 1: occupied
	int count = 0;

	size_t hash(const string& key) const{ //hash key
		size_t h = std::hash<string>{}(key);
		if(strat == 1){return h % buckets.size();}
		else{return h % bins.size();}
	}

	size_t hash2(const string& key) const{ //alternatively hash key
		size_t h = std::hash<string>{}(key);
		return 1 + (h % (bins.size() - 1));
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
			vector<vector<pair<string, Credentials>>> newBuckets{buckets.size() * 2};
			for(auto & bucket : buckets){
				for(auto& entry : bucket){
					const int newIndex = std::hash<string>{}(entry.first) % (newBuckets.size());
					newBuckets[newIndex].push_back(entry);
				}
			}
			buckets = std::move(newBuckets);
		}
		else{ //linear & quadratic probing & double hashing
			vector<pair<string, Credentials>> newBins(nextPrime(bins.size() * 2));
			vector<int> newState(newBins.size(), -1);
			for(int i = 0; i < bins.size(); i++){
				if(state[i]== 1){
					size_t start = std::hash<string>{}(bins[i].first) % newBins.size();
					size_t newIndex = 0;
					bool inserted = false;
					for(int j = 0; j < newBins.size(); j++){
						if(strat == 2){
							newIndex = (start + j) % newBins.size();
						}
						else if(strat == 3){
							newIndex = (start + j * j) % newBins.size();
						}
						else if(strat == 4){
							const size_t jump = 1 + (std::hash<string>{}(bins[i].first) % (newBins.size() - 1));
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
	void insert(const string& key, const Credentials& value){ //inserts a key and a value by into the bucket associated with the hashed key (auto resize + hash)
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
			}
			else{
				resish();
				insert(key, value);
				return;
			}
		}

		if(loadFactor() >= tolerance){
		    resish();
		}
	}

	optional<Credentials> find(const string& key) const{ //return the value paired with key using key
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
		return nullopt;
	}

	bool remove(const string& key){ //remove the entry using key
		int index = hash(key);
		if(strat == 1){ //separate chaining
			for(int i = 0; i < static_cast<int>(buckets[index].size()); i++){
				if(buckets[index][i].first == key){
					buckets[index].erase(buckets[index].begin() + i);
					count--;
					return true;
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
				if(state[index] == -1){
					break;
				}
				if(state[index] == 1 && bins[index].first == key){
					state[index] = 0;
					count--;
					return true;
				}
			}
		}
		return false;
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
		cout << "user login info table (username|hashedpassword):\n";
		if(strat == 1){

			for(int i = 0; i < static_cast<int>(buckets.size()); i++){
				bool first = true;
				cout << " bucket " << i << ": ";
				for(const auto& j : buckets[i]){
					if(!first) cout << ", ";
					cout << "(" << j.first << "|" << j.second.passwordHash << ")";
					first = false;
				}
				cout << endl;
			}
		}
		else{
			for(int i = 0; i < bins.size(); i++){
				cout << " bucket " << i << ": ";
				if(state[i] == 1) cout << "(" << bins[i].first << "|" << bins[i].second.passwordHash << ")";
				cout << endl;
			}
		}
		cout << " avg entry/bucket: " << loadFactor() << endl;
	}
};

static string login(const HTable& h, const string& user, const string& pass){
	bool logged = false;
	auto creds = h.find(user);
	if(creds && demoHash(creds -> salt + pass + pepper) == creds -> passwordHash){
		logged = true;
	}
	if(logged){
		return "login successful";
	}
	return "login failed";
}

int main(){
	HTable table;
	table.changeStrat(1);
	cout << "add 7 users\n";
	table.insert("br", createPass("passwurd"));
	table.insert("alice2", createPass("120122"));
	table.insert("bob", createPass("koasdmf"));
	table.insert("user0", createPass("ch1k3n"));
	table.insert("user2", createPass("passwurd"));
	table.insert("toby", createPass("ch1k3n"));
	table.insert("howardt", createPass("bethesda"));
	table.print();

	cout << "\nadd 2 users and remove 1 user\n";
	table.insert("cookieeasy", createPass("thegoat"));
	table.insert("higuy", createPass("greeting"));
	table.remove("user0");
	table.print();

	string res;
	do{
		string user, pass;
		cout << "\nusername: ";
		cin >> user;
		cout << "password: ";
		cin >> pass;
		res = login(table, user, pass);
		cout << res << endl;
	}while(res != "login successful");
}