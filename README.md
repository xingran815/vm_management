# Main logic

To achieve O(1) time complexity in `get_expenditure`, each user should record the total number of currently active vcpus to calculate the price of running jobs easily.

Data structure `list` + `unordered_map<int, list::iterator>` is used for O(1) deletion and O(1) loopup.

This question can be solved from the perspective of the data centre. 

To get the current price, each user should record four prices for O(1) price calculation: 
* price at last checked timestamp
* price for current active vms
* price for finished vms
* price for compensations

# Example

![VM diagram](vm_management.png)

the cost at `timestamp = 40s` is summed up from four parts:
1. the cost at last `timestamp = 20s`,
2. the cost of running vms `running_price`,
    ```C++
    running_price = total_vcpu * X * (40-20);
    ```
3. the cost of finished vms `finished_price` between `timestamp = 20s` and `timestamp = 40s`.
    whenever a job is stopped before `timestamp = 40s`

    ```C++
    finished_price += vcpu * X * (stop_ts-20);
    ```

4. (negative) the compensation when the vm is not running at `timestamp = 20s`
    whenever a job is started after `timestamp = 20s`

    ```C++
    compensation += vcpu * X * (start_ts-20);
    ```

# Solution
the solution is main.cpp