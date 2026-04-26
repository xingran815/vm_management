#include <iostream>
#include <string>
#include <iterator>
#include <unordered_map>
#include <list>
using namespace std;


// start_vm, stop_vm, and get_expenditure should be O(1)
// the current price is calculated as last_price + finished_job_price - compensation + running_job_price
class user_data {
    public:
        int user_id;
        int last_price_ts;      // timestamp when last_price was updated; valid only when initialized
        double last_price;      // price when last_price was updated
        double compensation;    // compensation when the vm starts after last_price_ts
        double finished_price;  // price for finished vms
        int cur_vcpus;          // current vcpus of the user
        bool initialized;       // false until the user's first start_vm sets last_price_ts

        user_data(int user_id) {
            this->user_id = user_id;
            this->last_price_ts = 0;
            this->last_price = 0;
            this->compensation = 0;
            this->finished_price = 0;
            this->cur_vcpus = 0;
            this->initialized = false;
        }
};
class vm_data {
    public:
        int vcpu;                           // vcpu for this vm
        list<user_data>::iterator p_user;   // pointing to the owner(user) of the vm
        vm_data(int vcpu, list<user_data>::iterator p_user) {
            this->vcpu = vcpu;
            this->p_user = p_user;
        }
};
class vm {
    public:
        double X;
        long long next_vm_seq;
        list<user_data> user_data_list;
        list<vm_data> vm_data_list;
        unordered_map<string, list<vm_data>::iterator> vm_id_map; // vm_id -> vm_data
        unordered_map<int, list<user_data>::iterator> user_id_map; // user_id -> user_data
        vm () {
            this->X = 1.0;
            this->next_vm_seq = 0;
        };
        string start_vm(int ts, int user_id, int vcpu) {
            // check if user_id is in user_id_map
            if (user_id_map.count(user_id) == 0) {
                // create new user_data
                user_data_list.push_back(user_data(user_id));
                user_id_map[user_id] = prev(user_data_list.end());
            }
            auto p_user = user_id_map[user_id];
            // first start for this user: anchor last_price_ts here so the
            // compensation delta is 0 (no over-charge for ts before the user existed)
            if (!p_user->initialized) {
                p_user->last_price_ts = ts;
                p_user->initialized = true;
            }
            // this vm starts after last price checking timestamp,
            // so the compensation should be calculated
            p_user->cur_vcpus += vcpu;
            p_user->compensation += (ts - p_user->last_price_ts)*vcpu*X;
            // create unique vm_id; the sequence suffix prevents collisions
            // when the same user starts multiple vms at the same ts with the same vcpu
            string vm_id = "user_" + to_string(user_id) +
                           "_ts_" + to_string(ts) +
                           "_vcpu_" + to_string(vcpu) +
                           "_seq_" + to_string(next_vm_seq++);
            vm_data_list.push_back(vm_data(vcpu, p_user));
            vm_id_map[vm_id] = prev(vm_data_list.end());
            return vm_id;
        }
        void stop_vm(int ts, string vm_id) {
            auto it = vm_id_map.find(vm_id);
            if (it == vm_id_map.end()) return;
            auto p_vm = it->second;
            int vcpu = p_vm->vcpu;
            auto p_user = p_vm->p_user;
            // update the price of finished vms
            p_user->finished_price += (ts-p_user->last_price_ts)*vcpu*X;
            p_user->cur_vcpus -= vcpu;
            // remove vm from the vm databank, list deletion is O(1)
            vm_data_list.erase(p_vm);
            vm_id_map.erase(it);
        }
        double get_expenditure(int ts, int user_id) {
            auto uit = user_id_map.find(user_id);
            if (uit == user_id_map.end()) return 0.0;
            auto p_user = uit->second;
            if (!p_user->initialized) return 0.0;
            // price calculation is O(1)
            // it is a sum up among 4 pars:
            // the running jobs, the finished jobs, the compensation, 
            // and the price of last checking ts
            double running_price = p_user->cur_vcpus * X * (ts - p_user->last_price_ts);
            double cur_price = running_price + p_user->last_price 
                                - p_user->compensation + p_user->finished_price;
            // cout << "running_price, last_price, compensation, finished_price, " << 
            //     running_price << ", " << p_user->last_price << ", " 
            //     << p_user->compensation << ", " << p_user->finished_price << endl;
            // reset the metadata
            p_user->last_price = cur_price;
            p_user->last_price_ts = ts;
            p_user->finished_price = 0.0;
            p_user->compensation = 0.0;
            return cur_price;
        }
};
int main() {
    vm my_vm;
    string vm_01 = my_vm.start_vm(1, 1, 2);
    cout << "started a vm for user 1 with 2 vcpus at ts 1. " << "vm_id: " << vm_01 << endl;
    my_vm.stop_vm(2,vm_01);
    cout << "stopped a vm for user 1 with 2 vcpus at ts 2. " << "vm_id: " << vm_01 << endl;
    cout << "the cost at ts 3 for user 1 is: " << my_vm.get_expenditure(3, 1) << endl;
    string vm_02 = my_vm.start_vm(4, 1, 4);
    cout << "started a vm for user 1 with 4 vcpus at ts 4. " << "vm_id: " << vm_02 << endl;
    string vm_03 = my_vm.start_vm(4, 1, 1);
    cout << "started a vm for user 1 with 1 vcpus at ts 4. " << "vm_id: " << vm_03 << endl;
    my_vm.stop_vm(6,vm_03);
    cout << "stopped a vm for user 1 with 1 vcpus at ts 6. " << "vm_id: " << vm_03 << endl;
    cout << "the cost at ts 7 for user 1 is: " << my_vm.get_expenditure(7, 1) << endl;
    string vm_04 = my_vm.start_vm(8, 2, 2);
    cout << "started a vm for user 2 with 2 vcpus at ts 8. " << "vm_id: " << vm_04 << endl;
    cout << "the cost at ts 9 for user 2 is: " << my_vm.get_expenditure(9, 2) << endl;
    cout << "the cost at ts 10 for user 1 is: " << my_vm.get_expenditure(10, 1) << endl;

    return 0;
}