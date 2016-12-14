#ifndef PROBABILITE_EXPONENTIAL_LAW_H
#define PROBABILITE_EXPONENTIAL_LAW_H

#include "evaluate_context.h"
#include "one_parameter_law.h"

namespace Probability {

class ExponentialLaw : public OneParameterLaw {
public:
  ExponentialLaw(EvaluateContext * evaluateContext);
  ~ExponentialLaw() override;
  const char * title() override;
  Expression * expression() const override;
  Type type() const override;
  bool isContinuous() override;
  float xMin() override;
  float yMin() override;
  float xMax() override;
  float yMax() override;
  const char * parameterNameAtIndex(int index) override;
  const char * parameterDefinitionAtIndex(int index) override;
private:
  Expression * m_expression;
};

}

#endif
