#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <condition_variable>
#include<bits/stdc++.h>

using namespace std;

class BankAccount {
private:
    int balance;
    mutex mtx;
    condition_variable cv;
    vector<string> transactionHistory;

    string getTimeStamp() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        stringstream timeStream;
        timeStream << setfill('0') << setw(2) << ltm->tm_hour << ":"
                   << setw(2) << ltm->tm_min << ":" << setw(2) << ltm->tm_sec;
        return timeStream.str();
    }

public:
    BankAccount() : balance(0) {}

    void deposit(int amount) {
        lock_guard<mutex> lock(mtx);
        balance += amount;
        string record = "[" + getTimeStamp() + "] 💰 Deposited: " + to_string(amount) + " | New Balance: " + to_string(balance);
        transactionHistory.push_back(record);
        cout << record << "\n";
        cv.notify_all();
    }

    void withdraw(int amount) {
        lock_guard<mutex> lock(mtx);
        if (amount > balance) {
            cout << "[" << getTimeStamp() << "] ❌ Withdrawal failed! Insufficient funds.\n";
            return;
        }
        balance -= amount;
        string record = "[" + getTimeStamp() + "] 💸 Withdrawn: " + to_string(amount) + " | New Balance: " + to_string(balance);
        transactionHistory.push_back(record);
        cout << record << "\n";
    }

    void withdrawWait(int amount) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&] { return balance >= amount; });
        balance -= amount;
        string record = "[" + getTimeStamp() + "] ⏳ Withdrawn (Waited): " + to_string(amount) + " | New Balance: " + to_string(balance);
        transactionHistory.push_back(record);
        cout << record << "\n";
    }

    void transfer(BankAccount &toAccount, int amount) {
        unique_lock<mutex> lock(mtx);
        if (amount > balance) {
            cout << "[" << getTimeStamp() << "] ❌ Transfer failed! Insufficient funds.\n";
            return;
        }
        balance -= amount;
        lock.unlock();
        toAccount.deposit(amount);
        lock.lock();
        string record = "[" + getTimeStamp() + "] 🔄 Transferred: " + to_string(amount) + " | New Balance: " + to_string(balance);
        transactionHistory.push_back(record);
        cout << record << "\n";
    }

    void checkBalance() {
        lock_guard<mutex> lock(mtx);
        cout << "[" << getTimeStamp() << "] 📊 Current Balance: " << balance << "\n";
    }

    void printTransactionHistory() {
        lock_guard<mutex> lock(mtx);
        cout << "\n📜 --- Transaction History ---\n";
        if (transactionHistory.empty()) {
            cout << "No transactions recorded yet.\n";
        } else {
            for (const string &record : transactionHistory) {
                cout << record << "\n";
            }
        }
    }
};

void singleThreadedMenu(BankAccount &account1, BankAccount &account2) {
    while (true) {
        cout << "\n💼 --- Menu ---\n";
        cout << "1️⃣ Deposit\n2️⃣ Withdraw\n3️⃣ Transfer\n4️⃣ Check Balance\n5️⃣ Print Transaction History\n6️⃣ Clear Screen\n7️⃣ Exit\n";
        cout << "🔹 Choose an option: ";
        int choice, amount;
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "💰 Enter amount to deposit: ";
                cin >> amount;
                account1.deposit(amount);
                break;
            case 2:
                cout << "💸 Enter amount to withdraw: ";
                cin >> amount;
                account1.withdraw(amount);
                break;
            case 3:
                cout << "🔄 Enter amount to transfer: ";
                cin >> amount;
                account1.transfer(account2, amount);
                break;
            case 4:
                account1.checkBalance();
                break;
            case 5:
                account1.printTransactionHistory();
                break;
            case 6:
                #ifdef _WIN32
                    system("CLS");
                #else
                    system("clear");
                #endif
                break;
            case 7:
                cout << "👋 Exiting...\n";
                return;
            default:
                cout << "⚠️ Invalid option! Try again.\n";
        }
    }
}

void multiThreadedSimulation(BankAccount &account) {
    cout << "\n🚀 Running Multi-Threaded Simulation...\n";

    auto depositTask = [&]() {
        for (int i = 0; i < 3; i++) {
            this_thread::sleep_for(chrono::milliseconds(500));
            account.deposit((rand() % 500) + 100);
        }
    };

    auto withdrawTask = [&]() {
        for (int i = 0; i < 3; i++) {
            this_thread::sleep_for(chrono::milliseconds(500));
            account.withdrawWait((rand() % 300) + 50);
        }
    };

    auto checkBalanceTask = [&]() {
        for (int i = 0; i < 3; i++) {
            this_thread::sleep_for(chrono::milliseconds(700));
            account.checkBalance();
        }
    };

    auto transactionHistoryTask = [&]() {
        this_thread::sleep_for(chrono::milliseconds(1000));
        account.printTransactionHistory();
    };

    thread t1(depositTask);
    thread t2(withdrawTask);
    thread t3(checkBalanceTask);
    thread t4(transactionHistoryTask);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

int main() {
    srand(time(0));
    BankAccount account1, account2;
    int choice;
    cout << "🔹 Choose mode:\n1️⃣ Single-threaded\n2️⃣ Multi-threaded Simulation\nEnter choice: ";
    cin >> choice;

    if (choice == 1) {
        singleThreadedMenu(account1, account2);
    } else if (choice == 2) {
        multiThreadedSimulation(account1);
    } else {
        cout << "⚠️ Invalid choice!\n";
    }

    return 0;
}