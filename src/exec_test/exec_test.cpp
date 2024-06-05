#include <iostream>
#include <map>
#include <iomanip>
#include <mod_coroutine/mod_cor.hpp>
#include <mod_coroutine/mod_cor_mutex.hpp>
#include <mod_time_wheel/mod_time_wheel.hpp>
#include <boost/asio/steady_timer.hpp>
#include <mod_atomic_queue/atomic_queue.hpp>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

static int test_button(int argc, const char* argv[]) {
    using namespace ftxui;
    int value = 50;

    // The tree of components. This defines how to navigate using the keyboard.
    auto buttons = Container::Horizontal({
                                                 Button("Decrease", [&] { value--; }),
                                                 Button("Increase", [&] { value++; }),
                                         });

    // Modify the way to render them on screen:
    auto component = Renderer(buttons, [&] {
        return vbox({
                            text("value = " + std::to_string(value)),
                            separator(),
                            gauge(value * 0.01f),
                            separator(),
                            buttons->Render(),
                    }) |
               border;
    });

    auto screen = ScreenInteractive::FitComponent();
    screen.Loop(component);
    return 0;
}

static int test_window(int argc, const char* argv[]) {
    using namespace ftxui;
    Element document = graph([](int x, int y) {
        std::vector<int> result(x, 0);
        for (int i{0}; i < x; ++i) {
            result[i] = ((3 * i) / 2) % y;
        }
        return result;
    });

    document |= color(Color::Red);
    document |= bgcolor(Color::DarkBlue);
    document |= border;

    const int width = 80;
    const int height = 10;
    auto screen =
            Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
    Render(screen, document);
    screen.Print();
    return EXIT_SUCCESS;
}

static int test_html_like(int argc, const char* argv[]) {
    using namespace ftxui;
    using namespace std::chrono_literals;

    auto img1 = []() { return text("img") | border; };
    auto img2 = []() { return vbox({text("big"), text("image")}) | border; };

    std::string reset_position;
    for (int i = 0;; ++i) {
        auto document =  //
                hflow(
                        paragraph("Hello world! Here is an image:"), img1(),
                        paragraph(" Here is a text "), text("underlined ") | underlined,
                        paragraph(" Here is a text "), text("bold ") | bold,
                        paragraph("Hello world! Here is an image:"), img2(),
                        paragraph(
                                "Le Lorem Ipsum est simplement du faux texte employé dans la "
                                "composition et la mise en page avant impression. Le Lorem "
                                "Ipsum est le faux texte standard de l'imprimerie depuis les "
                                "années 1500, quand un imprimeur anonyme assembla ensemble "
                                "des morceaux de texte pour réaliser un livre spécimen de "
                                "polices de texte. Il n'a pas fait que survivre cinq siècles, "
                                "mais s'est aussi adapté à la bureautique informatique, sans "
                                "que son contenu n'en soit modifié. Il a été popularisé dans "
                                "les années 1960 grâce à la vente de feuilles Letraset "
                                "contenant des passages du Lorem Ipsum, et, plus récemment, "
                                "par son inclusion dans des applications de mise en page de "
                                "texte, comme Aldus PageMaker."),
                        paragraph(" Here is a text "), text("dim ") | dim,
                        paragraph("Hello world! Here is an image:"), img1(),
                        paragraph(" Here is a text "), text("red ") | color(Color::Red),
                        paragraph(" A spinner "), spinner(6, i / 10)) |
                border;

        auto screen = Screen::Create(Dimension::Fit(document));
        Render(screen, document);
        std::cout << reset_position;
        screen.Print();
        reset_position = screen.ResetPosition();

        std::this_thread::sleep_for(0.01s);
    }

    return 0;
}

static int test_input2(int argc, const char* argv[])
{
    using namespace ftxui;

    std::string input_msg;
    Component msg_input_box;
    Elements display_msgs;
    float text_area_msg_cnt = 0;

    // 输入框
    InputOption input_option;
    input_option.on_enter = [&](){
        if (!input_msg.empty()) {
            display_msgs.push_back(hflow(paragraph(input_msg)));
            text_area_msg_cnt += 1;
            input_msg.clear();
        }
    };
    msg_input_box = Input(&input_msg, "", input_option);

    // 显示消息
    float taxt_area_cur_page = 0;
    auto get_text_area_pos = [&](){
        if (text_area_msg_cnt <= 0) {
            text_area_msg_cnt = 0;
            return .0f;
        }
        float h_per_line = 1.0f / text_area_msg_cnt;
        float h_page = 10 * h_per_line;
        float max_page_num = text_area_msg_cnt / 10;
        if (taxt_area_cur_page > max_page_num) {
            taxt_area_cur_page = max_page_num;
        }
        return (1.0f - (taxt_area_cur_page * h_page));
    };
    auto get_text_area = [&](){
        return vbox({ display_msgs });
    };

    auto get_msg_disp_box = [&](){
        return vbox({
            filler(),
            get_text_area() | focusPositionRelative(0, get_text_area_pos()) | vscroll_indicator | frame,
        });
    };

    auto component = Container::Vertical({
            msg_input_box,
        });
    auto get_while_page = [&]() {
        return vbox({
            get_msg_disp_box() | border | flex,
            hbox(text("Input: "), msg_input_box->Render()) | border,
        });
    };

    auto screen = ScreenInteractive::Fullscreen();
    auto ui_root_comp = Renderer(component, [&] {
        return get_while_page();
    });
    auto root_comp = CatchEvent(ui_root_comp, [&](Event event) {
        if (event == Event::PageUp) {
            taxt_area_cur_page += 1;
            return true;
        } else if (event == Event::PageDown) {
            taxt_area_cur_page -= 1;
            if (taxt_area_cur_page < 0) {
                taxt_area_cur_page = 0;
            }
            return true;
        }
        return false;
    });
    screen.Loop(root_comp);

    return 0;
}

static int test_input(int argc, const char* argv[]) {
    using namespace ftxui;

    std::string first_name;
    std::string last_name;
    std::string password;

    Component input_first_name = Input(&first_name, "first name");
    Component input_last_name = Input(&last_name, "last name");

    InputOption password_option;
    password_option.password = true;
    Component input_password = Input(&password, "password", password_option);

    auto component = Container::Vertical({
                                                 input_first_name,
                                                 input_last_name,
                                                 input_password,
                                         });

    auto renderer = Renderer(component, [&] {
        return vbox({
                            text("Hello " + first_name + " " + last_name),
                            separator(),
                            hbox(text(" First name : "), input_first_name->Render()),
                            hbox(text(" Last name  : "), input_last_name->Render()),
                            hbox(text(" Password   : "), input_password->Render()),
                    }) |
               border;
    });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(renderer);

    return 0;
}

static int test_pixel(int argc, const char* argv[])
{
    using namespace ftxui;
    auto screen = Screen::Create(Dimension::Fixed(32), Dimension::Fixed(10));

    auto& pixel = screen.PixelAt(9,9);
    pixel.character = U'A';
    pixel.bold = true;
    pixel.foreground_color = Color::Blue;

    std::cout << screen.ToString();
    return EXIT_SUCCESS;
}

static int test_box(int argc, const char* argv[])
{
    using namespace ftxui;

    // Define the document
    Element document =
            hbox({
                         text("left")   | border,
                         text("middle") | border | flex,
                         text("right")  | border,
                 });

    auto screen = Screen::Create(
            Dimension::Full(),       // Width
            Dimension::Fit(document) // Height
    );
    Render(screen, document);
    screen.Print();

    return EXIT_SUCCESS;
}

void async_sleep_thread(unsigned ts_us, std::function<void(int result)>&& cb) {
    cppt_usleep(ts_us);
    cb(rand() % ts_us);
}

void async_sleep(unsigned ts_us, std::function<void(int result)> cb)
{
    std::thread t(async_sleep_thread, ts_us, std::move(cb));
    t.detach();
}

int co_usleep(unsigned ts_us)
{
    int sleep_result = -1;
    auto wrap_func = [&](std::function<void(int result)>&& co_cb) {
        auto async_sleep_cb = [&, co_cb](int result) {
            co_cb(result);
        };
        async_sleep(ts_us, async_sleep_cb);
    };
    auto timeout_func = [&](){
        log_info("Time expired.");
    };
    sleep_result = cppt::cor_yield(wrap_func, 3000, timeout_func);
    return sleep_result;
}

void my_co1()
{
    log_info("1");
    log_info("result: %d", co_usleep(111));
    log_info("2");
    log_info("result: %d", co_usleep(222));
    log_info("3");
}

void my_co2(int n, cppt::cor_sp wait_co)
{
    wait_co->join();
    log_info("11: %d", n);
    log_info("result: %d", co_usleep(333));
    log_info("22");
    log_info("result: %d", co_usleep(444));
    log_info("33");
}

void my_co0()
{
    log_info("Start");
    co_usleep(5000000);
    auto co1 = cppt::cor_create(my_co1);
    cppt::cor_create(my_co2, 3, co1);
    co1->join();
    co1->join();
    co1->join();
    co_usleep(333);
    co1->join();
    co1->join();
    co1->join();
}

uint64_t fib(uint64_t n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

void cal_co()
{
    uint64_t n = 46;
    auto begin_ts_ms = util_now_ts_ms();
    uint64_t rst = fib(n);
    auto end_ts_ms = util_now_ts_ms();
    auto time_diff = end_ts_ms - begin_ts_ms;
    printf("fib %" PRIu64 ": %" PRIu64 "\n", n, rst);
    printf("time consume: %" PRIu64 "s%" PRIu64 "ms\n",
           time_diff / 1000, time_diff % 1000);
}

void cal_co_main()
{
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
    cppt::cor_create(cal_co);
}

void co_mutex_1(cppt::cor_mutex_t& mutex)
{
    log_info("1");
    log_info("2");
    mutex.lock();
    log_info("3");
    log_info("4");
    cppt_sleep(5);
    mutex.unlock();
    log_info("5");
    log_info("6");
}

void co_mutex_2(cppt::cor_mutex_t& mutex)
{
    log_info("a");
    log_info("b");
    mutex.lock();
    log_info("c");
    log_info("d");
    cppt_sleep(5);
    mutex.unlock();
    log_info("e");
    log_info("f");
}

void co_main_mutex()
{
    cppt::cor_mutex_t mutex;
    auto co1 = cppt::cor_create(co_mutex_1, std::ref(mutex));
    auto co2 = cppt::cor_create(co_mutex_2, std::ref(mutex));
    co1->join();
    co2->join();
}

static int test_cppt_co(int argc, const char* argv[])
{
    cppt::cor_create(my_co0);
//    cppt::cor_create(cal_co_main);
//    cppt::cor_create(co_main_mutex);
    cppt::cor_run();
    return 0;
}

class test_co_t {
public:
    void co_main(int i) {
        auto co1 = cppt::cor_create(my_co1);
        cppt::cor_create(my_co2, 3, co1);
        co1->join();
        co1->join();
        co1->join();
        co_usleep(333);
        co1->join();
        co1->join();
        co1->join();
    }
};

static int test_cppt_co_class(int argc, const char* argv[])
{
//    cppt::cor_create(my_co0);
//    cppt::cor_create(cal_co_main);
    test_co_t co;
    cppt::cor_create(&test_co_t::co_main, &co, 1);
    cppt::cor_run();
    return 0;
}

static int test_tw(int argc, const char* argv[])
{
    time_wheel_t time_wheel(60);

    time_wheel_task_t tw_task1(5, [](){
        log_info("Expired 1.");
    });
    time_wheel_task_t tw_task2(20, [](){
        log_info("Expired 2.");
    });

    time_wheel.add_tw_task(&tw_task1);
    time_wheel.add_tw_task(&tw_task2);

    boost::asio::io_context io;

    boost::asio::steady_timer timer(io);
    std::function<void(const boost::system::error_code&)> timer_cb = [&](const boost::system::error_code& ec){
        time_wheel.tick();
        timer.expires_after(boost::asio::chrono::seconds(1));
        timer.async_wait(timer_cb);
    };
    timer.async_wait(timer_cb);

    io.run();

    return 0;
}

class TestObject {
public:
    TestObject() {
        std::cout << "Object created in thread: " << std::this_thread::get_id() << std::endl;
    }

    ~TestObject() {
        std::cout << "Destructor called in thread: " << std::this_thread::get_id() << std::endl;
    }
};

int test_destructor(int argc, const char* argv[])
{
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;

    TestObject* obj = new TestObject();

    std::thread workerThread([&obj]() {
        std::cout << "Worker thread: " << std::this_thread::get_id() << std::endl;
        delete obj;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "2222222222 \n";
    });

    workerThread.join();

    return 0;
}

int test_sorted_map(int argc, const char* argv[])
{
    int xor_num = 9;

    // 替换比较函数，使得 map 按照 key 的异或值排序
    auto cmp = [&xor_num](int a, int b) {
        return (a ^ xor_num) < (b ^ xor_num);
    };
    std::map<int, std::string, decltype(cmp)> num_map(cmp);

    num_map.insert({1, "one"});
    num_map.insert({2, "two"});
    num_map.insert({3, "three"});
    num_map.insert({4, "four"});
    num_map.insert({5, "five"});
    num_map.insert({6, "six"});
    num_map.insert({7, "seven"});
    num_map.insert({8, "eight"});

    // 遍历 num_map
    for (auto & it : num_map) {
        // 以十六进制大写、保留两位数字、并且带着"0x"前缀的格式打印key的值
        std::cout << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << it.first << " ";
        // 以十六进制大写、保留两位数字、并且带着"0x"前缀的格式打印key与xor_num异或的值
        std::cout << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (it.first ^ xor_num) << " ";
        // 打印value的值
        std::cout << it.second << std::endl;
    }

    return 0;
}

int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

class para_queue_t {
public:
    struct ele_t {
        std::mutex value_lock;
        std::condition_variable value_cv;
        bool has_value = false;
        std::pair<int, int> value;
    };

    para_queue_t() : m_array_queue(m_queue_size) {}

    void push(std::pair<int, int>&& value, size_t value_number) {
        size_t cur_write_idx = value_number % m_queue_size;
        auto& ele = m_array_queue[cur_write_idx];
        std::unique_lock lock(ele.value_lock);
        ele.value_cv.wait(lock, [&]() {
            if (value_number - m_read_cnt >= m_queue_size) {
                return false;
            }
            if (ele.has_value) {
                return false;
            }
            return true;
        });
        ele.has_value = true;
        ele.value = std::move(value);
        ele.value_cv.notify_all();
    }

    std::pair<int, int> pop() {
        size_t cur_read_idx = m_read_cnt % m_queue_size;
        auto& ele = m_array_queue[cur_read_idx];
        std::unique_lock lock(ele.value_lock);
        ele.value_cv.wait(lock, [&]() { return ele.has_value; });
        ele.has_value = false;
        m_read_cnt++;
        ele.value_cv.notify_one();
        return std::move(ele.value);
    }

private:
    const size_t m_queue_size = 100;
    std::vector<ele_t> m_array_queue;
    size_t m_read_cnt = 0;
};

void worker_old(std::queue<std::pair<int, int>>& tasks, std::queue<std::pair<int, int>>& results, std::mutex& tasks_mutex, std::mutex& results_mutex, std::condition_variable& cv, int& next_result) {
    while (true) {
        std::unique_lock<std::mutex> task_lock(tasks_mutex);
        if (tasks.empty()) {
            return;
        }
        auto task = tasks.front();
        tasks.pop();
        task_lock.unlock();

        int task_number = task.first;
        int n = task.second;
        int result = fib(n);

        std::unique_lock<std::mutex> result_lock(results_mutex);
        cv.wait(result_lock, [&]() { return task_number == next_result; });
        results.push({task_number, result});
        next_result++;
        result_lock.unlock();
        cv.notify_all();
    }
}

struct Task {
    int id;
    int value;

    Task() : id(0), value(0) {}
    Task(int id, int value) : id(id), value(value) {}
};

bool validate_results(std::queue<std::pair<int, int>>& results) {
    // 预先计算的 10 到 19 的斐波那契值
    int fib_values[] = {55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181};

    int expected_task_number = 0;
    while (!results.empty()) {
        auto result = results.front();
        results.pop();

        // 检查任务编号是否按顺序排列
        if (result.first != expected_task_number) {
            std::cerr << "Error: Task number out of order. Expected " << expected_task_number << ", got " << result.first << '\n';
            return false;
        }

        // 检查结果值是否正确
        int expected_result = fib_values[result.first % 10]; // 从预先计算的数组中获取期望的结果
        if (result.second != expected_result) {
            std::cerr << "Error: Incorrect result for task number " << result.first << ". Expected " << expected_result << ", got " << result.second << '\n';
            return false;
        }

        expected_task_number++;
    }

    return true;
}

int test_fib_single_thr(int argc, const char* argv[])
{
    std::cout << "Preparing tasks...\n";

    // 创建任务队列
    std::queue<std::pair<int, int>> tasks;
    for (int i = 0; i < 10000000; i++) {
        int n = 10 + (i % 10); // 10 到 19 之间的数字
        tasks.push({i, n}); // 任务编号和要计算的数字
    }

    // 创建结果队列
    std::queue<std::pair<int, int>> results;

    std::cout << "Processing tasks...\n";

    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    // 执行任务
    while (!tasks.empty()) {
        auto task = tasks.front();
        tasks.pop();
        int task_number = task.first;
        int n = task.second;
        int result = fib(n);
        results.push({task_number, result});
    }

    // 记录结束时间
    auto stop = std::chrono::high_resolution_clock::now();

    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Total time taken by " << results.size() << " tasks: " << duration.count() << " milliseconds" << std::endl;

    // 验证结果
    std::cout << "Validating result...\n";
    if (validate_results(results)) {
        std::cout << "All results are correct and in order.\n";
    } else {
        std::cerr << "Validation failed.\n";
    }

    return 0;
}

int test_fib_multiple_thr(int argc, const char* argv[])
{
    std::cout << "Preparing tasks...\n";

    // 创建任务队列
    std::queue<std::pair<int, int>> tasks;
    for (int i = 0; i < 10000000; i++) {
        int n = 10 + (i % 10); // 10 到 19 之间的数字
        tasks.push({i, n}); // 任务编号和要计算的数字
    }

    // 创建结果队列
    std::queue<std::pair<int, int>> results;

    // 创建互斥锁和条件变量
    std::mutex tasks_mutex;
    std::mutex results_mutex;
    std::condition_variable cv;
    int next_result = 0;

    std::cout << "Processing tasks...\n";

    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    // 创建工作线程
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.push_back(std::thread(worker_old, std::ref(tasks), std::ref(results), std::ref(tasks_mutex), std::ref(results_mutex), std::ref(cv), std::ref(next_result)));
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 记录结束时间
    auto stop = std::chrono::high_resolution_clock::now();

    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Total time taken by " << results.size() << " tasks: " << duration.count() << " milliseconds" << std::endl;

    // 验证结果
    std::cout << "Validating result...\n";
    if (validate_results(results)) {
        std::cout << "All results are correct and in order.\n";
    } else {
        std::cerr << "Validation failed.\n";
    }

    return 0;
}

constexpr size_t queue_capacity = 1000000; // 队列容量
using TaskQueue = atomic_queue::AtomicQueue2<Task, queue_capacity>;

// 工作线程函数
void worker(TaskQueue& task_queue, para_queue_t& results) {
    Task task;
    while (task_queue.try_pop(task)) {
        int n = task.value;
        results.push({task.id, fib(n)}, task.id);
    }
}

int test_fib_multiple_thr_2(int argc, const char* argv[])
{
    const size_t num_tasks = 1000000; // 一百万个任务
    const size_t num_threads = 4;

    std::cout << "Preparing tasks...\n";

    // 创建任务队列并预先放入任务
    auto task_queue = std::make_unique<TaskQueue>();
    Task task;
    for (size_t i = 0; i < num_tasks; ++i) {
        task.id = i;
        task.value = 10 + (i % 10);
        task_queue->push(task);
    }

    // 创建结果队列
    para_queue_t results;

    std::cout << "Processing tasks...\n";

    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    // 创建工作线程
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(worker, std::ref(*task_queue), std::ref(results)));
    }

    // 提取结果
    std::queue<std::pair<int, int>> result_queue;
    size_t results_extracted = 0;
    while (results_extracted < num_tasks) {
        result_queue.push(results.pop());
        results_extracted++;
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 记录结束时间
    auto stop = std::chrono::high_resolution_clock::now();

    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Total time taken by " << result_queue.size() << " tasks: " << duration.count() << " milliseconds" << std::endl;

    // 验证结果
    std::cout << "Validating result...\n";
    if (validate_results(result_queue)) {
        std::cout << "All results are correct and in order.\n";
    } else {
        std::cerr << "Validation failed.\n";
    }

    return 0;
}

int test_fib(int argc, const char* argv[])
{
    test_fib_single_thr(argc, argv);
    test_fib_multiple_thr(argc, argv);
    test_fib_multiple_thr_2(argc, argv);
    return 0;
}

static int program_main(int argc, const char* argv[])
{
    int ret = -1;
//    ret = test_box(argc, argv);
//    ret = test_pixel(argc, argv);
//    ret = test_input(argc, argv);
//    ret = test_input2(argc, argv);
//    ret = test_html_like(argc, argv);
//    ret = test_window(argc, argv);
//    ret = test_button(argc, argv);
//    ret = test_cppt_co(argc, argv);
//    ret = test_cppt_co_class(argc, argv);
//    ret = test_tw(argc, argv);
//    ret = test_destructor(argc, argv);
//    ret = test_sorted_map(argc, argv);
    ret = test_fib(argc, argv);
    return ret;
}

int main(int argc, const char* argv[])
{
    int ret = -1;

    try {
        ret = program_main(argc, argv);
    } catch (const std::overflow_error& e) {
        std::cout << "overflow_error: " << e.what();
    } catch (const std::runtime_error& e) {
        std::cout << "runtime_error: " << e.what();
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what();
    } catch (...) {
        std::cout << "unknown exception!";
    }

    return ret;
}
