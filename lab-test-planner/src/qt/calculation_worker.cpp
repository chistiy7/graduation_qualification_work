#include "qt/calculation_worker.hpp"

#include "app/pipeline.hpp"
#include "app/preprocessor.hpp"
#include "app/scenario_runner.hpp"
#include "io/scenario_json.hpp"

namespace lab::qtui {

CalculationJobResult executeCalculation(bool customMode, const QString& presetId,
                                        const ScenarioInput& customInput) {
    CalculationJobResult result;
    try {
        if (customMode) {
            result.input = customInput;
            result.output = runUserScenario(customInput);
        } else {
            Pipeline pipeline;
            if (presetId == QStringLiteral("demo_8_types_80")) {
                result.output = pipeline.run(buildScenario8Types80());
            } else {
                Preprocessor preprocessor;
                result.output = pipeline.run(preprocessor.loadBuiltin(presetId.toStdString()));
            }
        }
        result.ok = true;
    } catch (const std::exception& ex) {
        result.error = QString::fromUtf8(ex.what());
    } catch (...) {
        result.error = QStringLiteral("Неизвестная ошибка расчёта.");
    }
    return result;
}

CalculationJobResult executeJsonScenario(const QString& jsonPath) {
    CalculationJobResult result;
    try {
        Pipeline pipeline;
        const auto bundle = loadScenarioJson(jsonPath.toStdString());
        result.output = pipeline.run(bundle);
        result.ok = true;
    } catch (const std::exception& ex) {
        result.error = QString::fromUtf8(ex.what());
    } catch (...) {
        result.error = QStringLiteral("Неизвестная ошибка загрузки или расчёта.");
    }
    return result;
}

}  // namespace lab::qtui
