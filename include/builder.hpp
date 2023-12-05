#ifndef PROTEI_COV_BUILDER_HPP
#define PROTEI_COV_BUILDER_HPP
#include "manager.hpp"
#include "config.hpp"
#include "threadpool.hpp"

class ManagerBuilder {
public:
    /**
     * @brief Метод для создания и настройки асинхронного логгера,
     * сохраняет логгер в локальную переменную.
     * @return Указатель на асинхронный логгер.
     */
    std::shared_ptr<spdlog::logger> BuildLogger();

    /**
     * @brief Метод для создания и настройки объекта конфигурации,
     * сохраняет объект конфигурации в переменную класса, устанавливает логгер конфигу.
     * @param pathToConfig Путь к файлу конфигурации.
     * @return Указатель на объект конфигурации.
     */
    std::shared_ptr<utility::ThreadSafeConfig> BuildConfig(const std::filesystem::path& pathToConfig);

    /**
     * @brief Метод для создания и настройки объектов записи данных CDR,
     * сохраняет вектор писателей в переменную класса.
     * @return возвращает массив указателей на объекты писателей
     */
    std::vector<std::shared_ptr<IRecorder>> BuildRecorders();
    /**
     * @brief Метод для создания и настройки объекта пула потоков,
     * устанавливает логгер пулу объектов.
     * @return Указатель на объект пула потоков.
     */
    std::shared_ptr<TP::ThreadPool> BuildThreadPool();

    /**
     * @brief Метод для конструирования объекта Manager с использованием созданных ранее компонентов,
     * подписывает менеджера на обновление конфигурации, устанавливает логгер менджеру.
     * @param pathToConfig Путь к файлу конфигурации.
     * @return Указатель на объект Manager.
     */
    std::shared_ptr<Manager> Construct(const std::filesystem::path& pathToConfig);
private:
    std::shared_ptr<spdlog::logger> logger; ///< указатель на асинхронный логгер
    std::shared_ptr<utility::ThreadSafeConfig> config; ///< указатель на конфиг
    std::vector<std::shared_ptr<IRecorder>> recorders; ///< вектор указателей на писателей
};

#endif // PROTEI_COV_BUILDER_HPP
