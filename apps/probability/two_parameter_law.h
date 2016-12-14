#ifndef PROBABILITE_TWO_PARAMETER_LAW_H
#define PROBABILITE_TWO_PARAMETER_LAW_H

#include "evaluate_context.h"
#include "law.h"

namespace Probability {

class TwoParameterLaw : public Law {
public:
  TwoParameterLaw(EvaluateContext * evaluateContext);
  virtual ~TwoParameterLaw() {};
  int numberOfParameter() override;
  float parameterValueAtIndex(int index) override;
  void setParameterAtIndex(float f, int index) override;
protected:
  float m_parameter1;
  float m_parameter2;
};

}

#endif
