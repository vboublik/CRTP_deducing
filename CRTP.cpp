#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
#include <memory>
#include <string>

// ================================================================
// 2.1. Шаблонний Метод
// ================================================================

namespace gof_21 {

    struct IReport {
        void generate() { header(); body(); footer(); }
        virtual ~IReport() = default;
    private:
        virtual void header() = 0;
        virtual void body() = 0;
        virtual void footer() = 0;
    };

    struct PdfReport : IReport {
    private:
        void header()   override { std::cout << "[PDF] header\n"; }
        void body()     override { std::cout << "[PDF] body\n"; }
        void footer()   override { std::cout << "[PDF] footer\n"; }
    };
    struct HtmlReport : IReport {
    private:
        void header()   override { std::cout << "[HTML] <head>\n"; }
        void body()     override { std::cout << "[HTML] <body>\n"; }
        void footer()   override { std::cout << "[HTML] </html>\n"; }
    };
    struct CsvReport : IReport {
    private:
        void header()   override { std::cout << "[CSV] col1,col2,col3\n"; }
        void body()     override { std::cout << "[CSV] 1,2,3\n"; }
        void footer()   override { std::cout << "[CSV] ---\n"; }
    };

} // namespace gof_21

namespace crtp_21 {

    template<typename Derived>
    struct Report {
        void generate() {
            static_cast<Derived*>(this)->header();
            static_cast<Derived*>(this)->body();
            static_cast<Derived*>(this)->footer();
        }
    };

    struct PdfReport : Report<PdfReport> {
        void header() { std::cout << "[PDF] header\n"; }
        void body() { std::cout << "[PDF] body\n"; }
        void footer() { std::cout << "[PDF] footer\n"; }
    };
    struct HtmlReport : Report<HtmlReport> {
        void header() { std::cout << "[HTML] <head>\n"; }
        void body() { std::cout << "[HTML] <body>\n"; }
        void footer() { std::cout << "[HTML] </html>\n"; }
    };
    struct CsvReport : Report<CsvReport> {
        void header() { std::cout << "[CSV] col1,col2,col3\n"; }
        void body() { std::cout << "[CSV] 1,2,3\n"; }
        void footer() { std::cout << "[CSV] ---\n"; }
    };

} // namespace crtp_21

namespace deduced_21 {

    struct Report {
        void generate(this auto& self) {
            self.header();
            self.body();
            self.footer();
        }
    };

    struct PdfReport : Report {
        void header() { std::cout << "[PDF] header\n"; }
        void body() { std::cout << "[PDF] body\n"; }
        void footer() { std::cout << "[PDF] footer\n"; }
    };
    struct HtmlReport : Report {
        void header() { std::cout << "[HTML] <head>\n"; }
        void body() { std::cout << "[HTML] <body>\n"; }
        void footer() { std::cout << "[HTML] </html>\n"; }
    };
    struct CsvReport : Report {
        void header() { std::cout << "[CSV] col1,col2,col3\n"; }
        void body() { std::cout << "[CSV] 1,2,3\n"; }
        void footer() { std::cout << "[CSV] ---\n"; }
    };

} // namespace deduced_21

template<typename... Reports>
struct Runner {
    std::tuple<Reports...> _reports;
    explicit Runner(Reports... rs) : _reports(std::move(rs)...) {}
    void run_all() {
        std::apply([](auto&... r) { (r.generate(), ...); }, _reports);
    }
};

// ================================================================
// 2.2. Null Object
// ================================================================

namespace crtp_22 {

    template<typename Derived>
    struct Logger {
        void log(const std::string& msg) {
            static_cast<Derived*>(this)->log_impl(msg);
        }
    };

    struct ConsoleLogger : Logger<ConsoleLogger> {
        void log_impl(const std::string& msg) {
            std::cout << "[LOG] " << msg << '\n';
        }
    };

    struct NullLogger : Logger<NullLogger> {
        void log_impl(const std::string&) {}
    };

} // namespace crtp_22

namespace deduced_22 {

    struct Logger {
        void log(this auto& self, const std::string& msg) {
            self.log_impl(msg);
        }
    };

    struct ConsoleLogger : Logger {
        void log_impl(const std::string& msg) {
            std::cout << "[LOG] " << msg << '\n';
        }
    };

    struct NullLogger : Logger {
        void log_impl(const std::string&) {}
    };

} // namespace deduced_22

template<typename L>
struct Service {
    L logger_;
    void process(const std::string& t) { logger_.log(t); }
};

// ================================================================
// 2.3. Спостерігач
// ================================================================

struct LoggerObs {
    void update(const auto& src) {
        std::cout << "[LOG] value=" << src.value << '\n';
    }
};
struct Display {
    void update(const auto& src) {
        std::cout << "[DISPLAY] value=" << src.value << '\n';
    }
};
struct Auditor {
    void update(const auto& src) {
        std::cout << "[AUDIT] value=" << src.value << '\n';
    }
};

namespace crtp_23 {

    template<typename Derived, typename... Observers>
    class Subject {
        std::tuple<Observers&...> observers_;
    public:
        explicit Subject(Observers&... obs) : observers_(obs...) {}
        void notify() {
            std::apply([this](auto&... obs) {
                (obs.update(static_cast<Derived&>(*this)), ...);
                }, observers_);
        }
    };

    struct EventSource : Subject<EventSource, LoggerObs, Display, Auditor> {
        using Subject::Subject;
        int value = 0;
        void fire(int v) { value = v; notify(); }
    };

} // namespace crtp_23

namespace deduced_23 {

    template<typename... Observers>
    class Subject {
        std::tuple<Observers&...> observers_;
    public:
        explicit Subject(Observers&... obs) : observers_(obs...) {}
        void notify(this auto& self) {
            std::apply([&self](auto&... obs) {
                (obs.update(self), ...);
                }, self.observers_);
        }
    };

    struct EventSource : Subject<LoggerObs, Display, Auditor> {
        using Subject::Subject;
        int value = 0;
        void fire(int v) { value = v; notify(); }
    };

} // namespace deduced_23

// ================================================================
// 2.4. Декоратор через Wrap
// ================================================================

template<class Base, template<class> class... Decorators>
struct Wrap;
template<class Base>
struct Wrap<Base> { using type = Base; };
template<class Base, template<class> class First, template<class> class... Rest>
struct Wrap<Base, First, Rest...> {
    using type = First<typename Wrap<Base, Rest...>::type>;
};

namespace crtp_24 {

    template<class Derived>
    struct Component {
        void op() { static_cast<Derived*>(this)->op_impl(); }
    };

    struct BaseComponent : Component<BaseComponent> {
        void op_impl() { std::cout << "Base\n"; }
    };

    template<class Inner>
    struct LoggingDecorator : Component<LoggingDecorator<Inner>> {
        Inner inner;
        void op_impl() {
            std::cout << "[log] before\n";
            inner.op();
            std::cout << "[log] after\n";
        }
    };

    template<class Inner>
    struct TimingDecorator : Component<TimingDecorator<Inner>> {
        Inner inner;
        void op_impl() {
            std::cout << "[time] start\n";
            inner.op();
            std::cout << "[time] stop\n";
        }
    };

    template<class Inner>
    struct RetryDecorator : Component<RetryDecorator<Inner>> {
        Inner inner;
        int   attempts = 3;
        void op_impl() {
            for (int i = 0; i < attempts; ++i) {
                std::cout << "[retry] attempt " << i + 1 << '\n';
                inner.op();
            }
        }
    };

    template<class Inner>
    using RetryDecorator3 = RetryDecorator<Inner>;

} // namespace crtp_24

namespace deduced_24 {

    struct Component {
        void op(this auto& self) { self.op_impl(); }
    };

    struct BaseComponent : Component {
        void op_impl() { std::cout << "Base\n"; }
    };

    template<class Inner>
    struct LoggingDecorator : Component {
        Inner inner;
        void op_impl() {
            std::cout << "[log] before\n";
            inner.op();
            std::cout << "[log] after\n";
        }
    };

    template<class Inner>
    struct TimingDecorator : Component {
        Inner inner;
        void op_impl() {
            std::cout << "[time] start\n";
            inner.op();
            std::cout << "[time] stop\n";
        }
    };

    template<class Inner>
    struct RetryDecorator : Component {
        Inner inner;
        int   attempts = 3;
        void op_impl() {
            for (int i = 0; i < attempts; ++i) {
                std::cout << "[retry] attempt " << i + 1 << '\n';
                inner.op();
            }
        }
    };

    template<class Inner>
    using RetryDecorator3 = RetryDecorator<Inner>;

} // namespace deduced_24

// ================================================================
// 2.5 Command через кортеж команд
// ================================================================

namespace crtp_25 {

    template<typename Derived>
    struct Command {
        void execute() {
            static_cast<Derived*>(this)->execute_impl();
        }
    };

    struct PrintHello : Command<PrintHello> {
        void execute_impl() { std::cout << "Hello\n"; }
    };
    struct PrintWorld : Command<PrintWorld> {
        void execute_impl() { std::cout << "World\n"; }
    };
    struct PrintDate : Command<PrintDate> {
        void execute_impl() { std::cout << "2026-06-22\n"; }
    };

} // namespace crtp_25

namespace deduced_25 {

    struct Command {
        void execute(this auto& self) {
            self.execute_impl();
        }
    };

    struct PrintHello : Command {
        void execute_impl() { std::cout << "Hello\n"; }
    };
    struct PrintWorld : Command {
        void execute_impl() { std::cout << "World\n"; }
    };
    struct PrintDate : Command {
        void execute_impl() { std::cout << "2026-06-22\n"; }
    };

} // namespace deduced_25

// Для обох

template<class... Cmds>
void execute_all(std::tuple<Cmds...>& t) {
    std::apply([](auto&&... xs) {
        (xs.execute(), ...);
        }, t);
}

// ================================================================
// main
// ================================================================

int main() {
    std::cout << "=== 2.1. Template method ===\n";

    std::cout << "\n--- GoF ---\n";
    std::vector<std::unique_ptr<gof_21::IReport>> gof_reports;
    gof_reports.push_back(std::make_unique<gof_21::PdfReport>());
    gof_reports.push_back(std::make_unique<gof_21::HtmlReport>());
    gof_reports.push_back(std::make_unique<gof_21::CsvReport>());
    for (auto& r : gof_reports) r->generate();

    std::cout << "\n--- CRTP ---\n";
    Runner crtp_runner{
        crtp_21::PdfReport{}, crtp_21::HtmlReport{}, crtp_21::CsvReport{} };
    crtp_runner.run_all();

    std::cout << "\n--- Deducing this ---\n";
    Runner deduced_runner{
        deduced_21::PdfReport{}, deduced_21::HtmlReport{}, deduced_21::CsvReport{} };
    deduced_runner.run_all();

    std::cout << "\n=== 2.2. Null Object ===\n";

    std::cout << "\n--- CRTP ---\n";
    Service<crtp_22::ConsoleLogger>  real_crtp;
    Service<crtp_22::NullLogger>     silent_crtp;
    real_crtp.process("connecting...");
    silent_crtp.process("connecting...");

    std::cout << "\n--- Deducing this ---\n";
    Service<deduced_22::ConsoleLogger> real_deduced;
    Service<deduced_22::NullLogger>    silent_deduced;
    real_deduced.process("connecting...");
    silent_deduced.process("connecting...");

    std::cout << "\n=== 2.3. Obserever ===\n";

    LoggerObs log; Display disp; Auditor aud;

    std::cout << "\n--- CRTP ---\n";
    crtp_23::EventSource crtp_src{ log, disp, aud };
    crtp_src.fire(42);

    std::cout << "\n--- Deducing this ---\n";
    deduced_23::EventSource deduced_src{ log, disp, aud };
    deduced_src.fire(42);

    std::cout << "\n=== 2.4. Decorator ===\n";

    std::cout << "\n--- CRTP ---\n";
    using CRTPChain = typename Wrap<crtp_24::BaseComponent,
        crtp_24::LoggingDecorator,
        crtp_24::TimingDecorator,
        crtp_24::RetryDecorator3>::type;
    CRTPChain crtp_obj;
    crtp_obj.op();

    std::cout << "\n--- Deducing this ---\n";
    using DeducedChain = typename Wrap<deduced_24::BaseComponent,
        deduced_24::LoggingDecorator,
        deduced_24::TimingDecorator,
        deduced_24::RetryDecorator3>::type;
    DeducedChain deduced_obj;
    deduced_obj.op();

    std::cout << "\n=== 2.5. Command ===\n";

    std::cout << "\n--- CRTP ---\n"; 
    auto crtp_cmds = std::make_tuple(
        crtp_25::PrintHello{},
        crtp_25::PrintWorld{},
        crtp_25::PrintDate{});
    execute_all(crtp_cmds);

    std::cout << "\n--- Deducing this ---\n"; 
    auto deduced_cmds = std::make_tuple(
        deduced_25::PrintHello{},
        deduced_25::PrintWorld{},
        deduced_25::PrintDate{});
    execute_all(deduced_cmds);

    return 0;
}