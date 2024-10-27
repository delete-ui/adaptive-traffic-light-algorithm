#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <random>
#include <stdexcept>

// Максимальное время зеленого сигнала
constexpr double MAX_GREEN_TIME = 60.0;
constexpr int CHECK_INTERVAL = 1000; // Интервал проверки в миллисекундах

constexpr double VEHICLE_WEIGHT = 0.7;
constexpr double PEDESTRIAN_WEIGHT = 0.3;

class TrafficLight
{
public:
    TrafficLight(int id) : id(id), vehicle_queue(0), pedestrian_queue(0), green_time(0) {}

    void receiveEvent(const std::string& type, int count)
    {
        // Валидация входящих данных
        if (count < 0)
        {
            throw std::invalid_argument("Count cannot be negative");
        }

        if (type == "vehicle")
        {
            vehicle_queue += count;
        }
        else if (type == "pedestrian")
        {
            pedestrian_queue += count;
        }
        else
        {
            throw std::invalid_argument("Invalid event type");
        }
    }

    std::pair<int, double> sendEvent() const
    {
        return { id, calculatePriority() };
    }

    void setGreenTime(double time)
    {
        green_time = time;
    }

    double calculatePriority() const
    {
        return vehicle_queue * VEHICLE_WEIGHT + pedestrian_queue * PEDESTRIAN_WEIGHT;
    }

    void resetQueues()
    {
        vehicle_queue = 0;
        pedestrian_queue = 0;
    }

    int getId() const
    {
        return id;
    }

    void logStatus() const
    {
        std::cout << "|ID: " << id << "|Vehicles: " << vehicle_queue << "|Pedestrians: " << pedestrian_queue << "|Priority: " << calculatePriority() << std::endl << std::endl;
    }

    static std::vector<std::pair<int, double>> determinePriority(const std::vector<TrafficLight>& traffic_lights)
    {
        std::vector<std::pair<int, double>> priorities;

        for (const auto& light : traffic_lights)
        {
            priorities.push_back(light.sendEvent());
        }

        std::sort(priorities.begin(), priorities.end(), [](const auto& a, const auto& b)
            {
                return a.second > b.second;
            });

        return priorities;
    }

    static void updateLightTimings(std::vector<TrafficLight>& traffic_lights)
    {
        auto priorities = determinePriority(traffic_lights);
        double total_priority = 0.0;

        for (const auto& [id, priority] : priorities)
        {
            total_priority += priority;
        }

        for (auto& light : traffic_lights)
        {
            double current_priority = 0.0;
            for (const auto& [id, priority] : priorities)
            {
                if (id == light.getId())
                {
                    current_priority = priority;
                    break;
                }
            }

            double green_time = (current_priority / total_priority) * MAX_GREEN_TIME;
            light.setGreenTime(green_time);
        }
    }

    static void receiveAndBroadcastEvents(std::vector<TrafficLight>& traffic_lights)
    {
        // Используем генератор случайных чисел для эмуляции событий
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 20);

        for (auto& light : traffic_lights)
        {
            light.receiveEvent("vehicle", distr(eng));
            light.receiveEvent("pedestrian", distr(eng));
        }
    }

private:
    int id;
    int vehicle_queue;
    int pedestrian_queue;
    double green_time;
};

int main()
{
    std::vector<TrafficLight> traffic_lights;

    for (int i = 0; i < 4; ++i)
    {
        traffic_lights.emplace_back(i);
    }

    try
    {
        // Основной цикл обработки трафика
        while (true)
        {
            TrafficLight::receiveAndBroadcastEvents(traffic_lights); 
            TrafficLight::updateLightTimings(traffic_lights);

            // Логирование текущих статусов для отладки или мониторинга
            for (const auto& light : traffic_lights)
            {
                light.logStatus();
            }

            // Сброс очередей для следующего цикла
            for (auto& light : traffic_lights)
            {
                light.resetQueues();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_INTERVAL));
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}