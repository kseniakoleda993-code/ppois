#pragma once

#include <string>
#include <utility>

#include "Common.h"
#include "Services.h"

namespace autoshop {

inline constexpr const char* kDieselEngine = "diesel";
inline constexpr const char* kPetrolEngine = "petrol";
inline constexpr int kDieselOilIntervalKm = 10000;
inline constexpr int kPetrolOilIntervalKm = 15000;
inline constexpr int kHighPerformancePowerHp = 250;

/// Двигатель автомобиля.
class Engine {
public:
    Engine(std::string type, double volumeLiters, int powerHp)
        : type_(std::move(type)), volumeLiters_(volumeLiters), powerHp_(powerHp) {}

    const std::string& getType() const { return type_; }
    double getVolumeLiters() const { return volumeLiters_; }
    int getPowerHp() const { return powerHp_; }
    bool isDiesel() const { return type_ == kDieselEngine; }
    bool isHighPerformance() const { return powerHp_ >= kHighPerformancePowerHp; }
    int oilChangeIntervalKm() const { return isDiesel() ? kDieselOilIntervalKm : kPetrolOilIntervalKm; }

private:
    std::string type_;
    double volumeLiters_;
    int powerHp_;
};

inline constexpr double kTireReplacementWear = 80.0;
inline constexpr double kMaxWearPercent = 100.0;

/// Комплект шин.
class TireSet {
public:
    TireSet(std::string size, std::string season, double wearPercent, int productionYear)
        : size_(std::move(size)), season_(std::move(season)), wearPercent_(wearPercent),
          productionYear_(productionYear) {}

    const std::string& getSize() const { return size_; }
    const std::string& getSeason() const { return season_; }
    double getWearPercent() const { return wearPercent_; }
    int getProductionYear() const { return productionYear_; }
    bool needsReplacement() const { return wearPercent_ >= kTireReplacementWear; }
    void addWear(double percent) { wearPercent_ = std::min(kMaxWearPercent, wearPercent_ + percent); }
    int ageInYears(int currentYear) const { return currentYear - productionYear_; }

private:
    std::string size_;
    std::string season_;
    double wearPercent_;
    int productionYear_;
};

/// Страховой полис.
class Insurance {
public:
    Insurance(std::string policyNumber, std::string provider, const Date& validUntil, const Money& coverageLimit)
        : policyNumber_(std::move(policyNumber)), provider_(std::move(provider)), validUntil_(validUntil),
          coverageLimit_(coverageLimit) {}

    const std::string& getPolicyNumber() const { return policyNumber_; }
    const std::string& getProvider() const { return provider_; }
    const Date& getValidUntil() const { return validUntil_; }
    const Money& getCoverageLimit() const { return coverageLimit_; }
    bool isValidOn(const Date& date) const { return !validUntil_.isBefore(date); }
    bool covers(const Money& damage) const { return coverageLimit_.isGreaterOrEqual(damage); }
    void extend(int days) { validUntil_ = validUntil_.plusDays(days); }

private:
    std::string policyNumber_;
    std::string provider_;
    Date validUntil_;
    Money coverageLimit_;
};

/// Гарантия на автомобиль или выполненные работы.
class Warranty {
public:
    Warranty(std::string description, const Date& coveredUntil, int mileageLimitKm)
        : description_(std::move(description)), coveredUntil_(coveredUntil), mileageLimitKm_(mileageLimitKm) {}

    const std::string& getDescription() const { return description_; }
    const Date& getCoveredUntil() const { return coveredUntil_; }
    int getMileageLimitKm() const { return mileageLimitKm_; }
    bool isActive(int mileageKm, const Date& date) const {
        return !coveredUntil_.isBefore(date) && mileageKm <= mileageLimitKm_;
    }
    int remainingKm(int mileageKm) const { return std::max(0, mileageLimitKm_ - mileageKm); }
    void prolong(int days) { coveredUntil_ = coveredUntil_.plusDays(days); }

private:
    std::string description_;
    Date coveredUntil_;
    int mileageLimitKm_;
};

inline constexpr std::size_t kVinLength = 17;
inline constexpr int kMinVehicleYear = 1950;

/// Автомобиль клиента.
class Vehicle {
public:
    Vehicle(std::string vin, std::string plate, std::string make, std::string model, int year, int mileage,
            const Engine& engine, const TireSet& tires)
        : vin_(std::move(vin)), plate_(std::move(plate)), make_(std::move(make)), model_(std::move(model)),
          year_(year), mileage_(mileage), engine_(engine), tires_(tires) {
        if (vin_.size() != kVinLength || plate_.empty() || year_ < kMinVehicleYear || mileage_ < 0) {
            throw InvalidVehicleDataException(vin_);
        }
    }

    const std::string& getVin() const { return vin_; }
    const std::string& getPlate() const { return plate_; }
    const std::string& getMake() const { return make_; }
    const std::string& getModel() const { return model_; }
    int getYear() const { return year_; }
    int getMileage() const { return mileage_; }
    int getOwnerId() const { return ownerId_; }
    const Engine& getEngine() const { return engine_; }
    const TireSet& getTires() const { return tires_; }
    void assignOwnerId(int clientId) { ownerId_ = clientId; }
    void driveKm(int km) {
        if (km < 0) {
            throw InvalidVehicleDataException("отрицательный пробег");
        }
        mileage_ += km;
    }
    void updateMileage(int newMileage) {
        if (newMileage < mileage_) {
            throw InvalidVehicleDataException("пробег не может уменьшаться");
        }
        mileage_ = newMileage;
    }
    int ageInYears(int currentYear) const { return currentYear - year_; }
    void replaceTires(const TireSet& newTires) { tires_ = newTires; }
    bool needsTireReplacement() const { return tires_.needsReplacement(); }
    bool isInsuredOn(const Insurance& insurance, const Date& date) const { return insurance.isValidOn(date); }
    bool isUnderWarranty(const Warranty& warranty, const Date& date) const {
        return warranty.isActive(mileage_, date);
    }

private:
    std::string vin_;
    std::string plate_;
    std::string make_;
    std::string model_;
    int year_;
    int mileage_;
    int ownerId_ = 0;
    Engine engine_;
    TireSet tires_;
};

/// План регулярного обслуживания: через сколько километров нужна услуга.
class ServicePlan {
public:
    ServicePlan(std::string serviceName, int intervalKm, int lastServiceKm)
        : serviceName_(std::move(serviceName)), intervalKm_(intervalKm), lastServiceKm_(lastServiceKm) {}

    const std::string& getServiceName() const { return serviceName_; }
    int getIntervalKm() const { return intervalKm_; }
    int getLastServiceKm() const { return lastServiceKm_; }
    int kmUntilDue(const Vehicle& vehicle) const { return lastServiceKm_ + intervalKm_ - vehicle.getMileage(); }
    bool isDue(const Vehicle& vehicle) const { return kmUntilDue(vehicle) <= 0; }
    void recordService(const Vehicle& vehicle) { lastServiceKm_ = vehicle.getMileage(); }

private:
    std::string serviceName_;
    int intervalKm_;
    int lastServiceKm_;
};

inline constexpr int kCriticalSeverity = 4;
inline constexpr int kMaxSeverity = 5;

/// Результат диагностики одного узла автомобиля.
class DiagnosticResult {
public:
    DiagnosticResult(std::string code, std::string description, int severity, const Money& estimatedCost)
        : code_(std::move(code)), description_(std::move(description)), severity_(severity),
          estimatedCost_(estimatedCost) {}

    const std::string& getCode() const { return code_; }
    const std::string& getDescription() const { return description_; }
    int getSeverity() const { return severity_; }
    const Money& getEstimatedCost() const { return estimatedCost_; }
    bool isCritical() const { return severity_ >= kCriticalSeverity; }
    int priority() const { return kMaxSeverity - severity_; }

private:
    std::string code_;
    std::string description_;
    int severity_;
    Money estimatedCost_;
};

}  // namespace autoshop
