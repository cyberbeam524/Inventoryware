// https://github.com/mtrebi/thread-pool/blob/master/README.md#submit-function
// 9.1
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
#include <iostream>
#include <boost/dll.hpp>

using namespace std;

#include <cstdint>
#include <iostream>
#include <vector>
# include <fstream>  
# include <stdio.h>  
# include <string.h>  
#include <mutex>
#include <pqxx/pqxx>
#include <memory>
#include <condition_variable>
#include <queue>
#include <memory>

using namespace std;  

const std::string HOST = "127.0.0.1";
const std::string PORT = "5434";
const std::string DB_NAME = "postgres";
const std::string USER = "admin";
const std::string PASSWORD = "admin";
const std::string SSL_MODE = "";
const std::string SSL_ROOT_CERT = "";

// thread safe queue for tasks
template<typename T>
class thread_safe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    thread_safe_queue()
    {}

    int size(){
        return data_queue.size();
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk,[this]{return !data_queue.empty();});
    value=std::move(data_queue.front());
    data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk,[this]{return !data_queue.empty();});
    std::shared_ptr<T> res(
    std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
    return res; 
    }
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value=std::move(data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
        return std::shared_ptr<T>();
        std::shared_ptr<T> res(
            std::make_shared<T>(std::move(data_queue.front())));
            data_queue.pop();
            return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

// join_threads joiner class
class join_threads
{
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_):
        threads(threads_)
    {}
    ~join_threads()
    {
        for(unsigned long i=0;i<threads.size();++i)
        {
            if(threads[i].joinable())
                threads[i].join();
} }
};

// worker threadpool
class thread_pool{
    std::atomic_bool done;
    thread_safe_queue<std::function<void()> > work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;
    void worker_thread() {
        while(!done) {
            std::function<void()> task;
            if(work_queue.try_pop(task)){

            task(); 
            }else{
                std::this_thread::yield();
            } }
        }

public:
    thread_pool(int count):
        
    done(false),joiner(threads){
        unsigned const thread_count=count;
        try {

            for(unsigned i=0;i<thread_count;++i){
                cout << "pushing " << i << "\n";
                threads.push_back(
                    std::thread(&thread_pool::worker_thread,this));
                };
        } 
        catch(int myNum) {
            done=true;
            throw; 
            }
        }

    ~thread_pool() { done=true; }

    template<typename FunctionType>
    void submit(FunctionType f){
        work_queue.push(std::function<void()>(f));
    }

    template<typename FunctionType, typename...Args>
    auto submit_with_args(FunctionType&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<FunctionType>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign 
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        std::function<void()> wrapper_func = [task_ptr]() {
        (*task_ptr)(); 
        };

        work_queue.push(std::function<void()>(wrapper_func));
        // Return future from promise
        return task_ptr->get_future();
    }

    
};

struct InventoryItem{
    string name;
    string expiryName; 
    int quantity;
    };

class PGPool {
private:
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<std::unique_ptr<pqxx::connection>> m_pool;
    int POOL_SIZE;

    void createPool(int thread_count) {
        this->POOL_SIZE = thread_count;
        std::lock_guard<std::mutex> locker_( m_mutex );
        this->m_pool = std::queue<std::unique_ptr<pqxx::connection>>();
        std::string url = "host=" + HOST + " port=" + PORT + " dbname=" + DB_NAME + " user=" + USER + " password=" + PASSWORD;
        if (SSL_MODE != "") {
            url += " sslmode=" + SSL_MODE;
            if (SSL_ROOT_CERT != "") { url += " sslrootcert=" + SSL_ROOT_CERT; }
        }
        std::cout << ">>>> Connecting to PostgresDB!" << std::endl;
        for (auto i=0; i < POOL_SIZE; i++) {
            m_pool.emplace(std::make_unique<pqxx::connection>(pqxx::connection(url)));
        }
        }

    

public:
    PGPool(int thread_count) {
        createPool(thread_count); 
        }

    int size() const{
            std::lock_guard<std::mutex> lock_(m_mutex);
            return m_pool.size();
        }

    std::unique_ptr<pqxx::connection> connection() {
        std::unique_lock<std::mutex> lock_(m_mutex);
        // if pool is empty, then wait until it notifies back
        while (m_pool.empty()) { m_condition.wait(lock_); }
        // get new connection in queue
        auto conn_ = std::move(m_pool.front());
        // immediately pop as we will use it now
        m_pool.pop();
        return conn_;
        }

    void freeConnection(std::unique_ptr<pqxx::connection> conn_) {
        std::unique_lock<std::mutex> lock_(m_mutex);
        // push a new connection into a pool
        m_pool.push(std::move(conn_));
        // unlock mutex
        lock_.unlock();
        // notify one of thread that is waiting
        m_condition.notify_one();
        }
};
#include <cstdlib>

int main()
{
    string dbName = "mydb"; string tableName = "inventory"; int option = 0;
    cout << "point 9\n";
    // unsigned const thread_count=std::thread::hardware_concurrency();
    unsigned const thread_count=95;
    cout << "Best number of worker pool threads to prevent context switching: " << thread_count << "\n";
    thread_pool workerpool = thread_pool(thread_count);

    cout << "point 3\n";
    PGPool pool = PGPool(thread_count + 5);
    cout << "starting pool with initial pool size: " << pool.size() << "\n";
    cout << "--------- point 0 \n";
    auto connectionpoolFunc3 = [](string taskName, int one, std::reference_wrapper<PGPool*> q) {
            auto conn2 = q.get()->connection();
            pqxx::work txn(*conn2);
            // cout << taskName << " -1 connetion status: " << conn2->is_open() << "--- " << q.get()->size() << "\n";
            txn.exec("DROP TABLE IF EXISTS DemoAccount");
            txn.exec("CREATE TABLE DemoAccount ( \
                        id int PRIMARY KEY, \
                        name varchar, \
                        age int, \
                        country varchar, \
                        balance int)");
            txn.exec("INSERT INTO DemoAccount VALUES \
                        (1, 'Jessica', 28, 'USA', 20000), \
                        (2, 'John', 28, 'Canada', 9000)");
            txn.commit();
            // cout << taskName << " -2connetion status: " << conn2->is_open() << "--- " << q.get()->size() << "\n";
            // move unique pointer back to thread safe queue without 
            // assigning it to anything else -- which will make it self destruct
            q.get()->freeConnection(std::move(conn2));
            // cout << ">>>> Successfully created table DemoAccount."  + taskName << std::endl;
            return "Executed " + taskName;
        };
        auto connectionpoolFunc4 = [](string taskName, int one, std::reference_wrapper<PGPool*> q) {
            // cout << "pool.size at start: " << q.get()->size() << "\n";
            auto conn2 = q.get()->connection();
            pqxx::work txn(*conn2);
            // cout << taskName << " -1 connetion status: " << conn2->is_open() << "--- " << q.get()->size() << "\n";
            txn.exec("DROP TABLE IF EXISTS DemoAccount2");
            txn.exec("CREATE TABLE DemoAccount2 ( \
                            id int PRIMARY KEY, \
                            name varchar, \
                            age int, \
                            country varchar, \
                            balance int)");
            txn.exec("INSERT INTO DemoAccount2 VALUES \
                        (1, 'Jessica', 28, 'USA', 20000), \
                        (2, 'Jessica', 28, 'USA', 20000), \
                        (3, 'Jessica', 28, 'USA', 20000), \
                        (4, 'Jessica', 28, 'USA', 20000), \
                        (5, 'John', 28, 'Canada', 9000)");

            txn.exec("DROP TABLE IF EXISTS DemoAccount");

            txn.exec("CREATE TABLE DemoAccount ( \
                        id int PRIMARY KEY, \
                        name varchar, \
                        age int, \
                        country varchar, \
                        balance int)");
            txn.exec("INSERT INTO DemoAccount VALUES \
                        (1, 'Jessica', 28, 'USA', 20000), \
                        (2, 'John', 28, 'Canada', 9000)");
            txn.commit();

            // sleep function to simulate a lengthier function executing:
            usleep(3000000);
            // cout << taskName << " -2connetion status: " << conn2->is_open() << "--- " << q.get()->size() << "\n";
            // std::cout << ">>>> Successfully created table DemoAccount."  + taskName << std::endl;
            q.get()->freeConnection(std::move(conn2));
            // cout << "pool.size at end: " << q.get()->size() << "\n";
            return "Executed " + taskName;};

        vector<std::future<string>> results;
        int random = rand();
        cout << "before starting work : pool.size: " << pool.size() << "\n";
        for (int i = 0; i<10;i++){
            random = rand();
            PGPool* threadLocalPool = &pool;
            if (random % 2 == 0)
                results.push_back(std::move(workerpool.submit_with_args(connectionpoolFunc3, "task" + to_string(i), 1, std::ref(threadLocalPool) )));
            else
                results.push_back(std::move(workerpool.submit_with_args(connectionpoolFunc4, "task" + to_string(i), 1, std::ref(threadLocalPool) )));
            // cout << "pool.size: " << pool.size() << "\n";
        }

        // getting all future results here
        for (int j = 0; j<results.size(); j++) cout << "Completion status: " << results[j].get() << "\n";


    if(const char* env_p = std::getenv("MMMM"))
        std::cout << "Your PATH is: " << env_p << '\n';
}
