#include "unit_list_controller.h"
#include "../app.h"
#include "../../shared/poincare_helpers.h"
#include <poincare/unit_convert.h>
#include <poincare/multiplication.h>
#include <poincare/power.h>
#include <poincare/undefined.h>
#include <poincare/unit.h>

using namespace Poincare;

namespace Calculation {

void UnitListController::setExpression(Poincare::Expression e) {
  // TODO: order of call issue!!!
  ExpressionsListController::setExpression(e);
  assert(!m_expression.isUninitialized());
  // Reinitizlize m_memoizedExpressions
  for (size_t i = 0; i < k_maxNumberOfCells; i++) {
    m_memoizedExpressions[i] = Expression();
  }

  size_t numberOfMemoizedExpressions = 0;
  // 1. First rows: miscellaneous classic units for some dimensions
  ExpressionNode::ReductionContext reductionContext(
      App::app()->localContext(),
      Preferences::sharedPreferences()->complexFormat(),
      Preferences::sharedPreferences()->angleUnit(),
      ExpressionNode::ReductionTarget::User,
      ExpressionNode::SymbolicComputation::ReplaceAllSymbolsWithDefinitionsOrUndefined);
  Expression copy = m_expression.clone();
  Expression units;
  // Reduce to be able to recognize units
  copy = copy.reduce(reductionContext);
  copy = copy.removeUnit(&units);
  bool requireSimplification = false;
  bool canChangeUnitPrefix = false;

  if (Unit::IsISSpeed(units)) {
    // 1.a. Turn speed into km/h
    m_memoizedExpressions[numberOfMemoizedExpressions++] = UnitConvert::Builder(
        m_expression.clone(),
        Multiplication::Builder(
          Unit::Kilometer(),
          Power::Builder(
            Unit::Hour(),
            Rational::Builder(-1)
            )
          )
        );
    requireSimplification = true; // Reduce the conversion
  } else if (Unit::IsISVolume(units)) {
    // 1.b. Turn volume into L
    m_memoizedExpressions[numberOfMemoizedExpressions++] = UnitConvert::Builder(
        m_expression.clone(),
        Unit::Liter()
        );
    requireSimplification = true; // reduce the conversion
    canChangeUnitPrefix = true; // Pick best prefix (mL)
  } else if (Unit::IsISEnergy(units)) {
    // 1.c. Turn energy into Wh
    m_memoizedExpressions[numberOfMemoizedExpressions++] = UnitConvert::Builder(
        m_expression.clone(),
        Multiplication::Builder(
          Unit::Watt(),
          Unit::Hour()
          )
        );
    m_memoizedExpressions[numberOfMemoizedExpressions++] = UnitConvert::Builder(
        m_expression.clone(),
        Unit::ElectronVolt()
        );
    requireSimplification = true; // reduce the conversion
    canChangeUnitPrefix = true; // Pick best prefix (kWh)
  } else if (Unit::IsISTime(units)) {
    // Turn time into ? year + ? month + ? day + ? h + ? min + ? s
    double value = Shared::PoincareHelpers::ApproximateToScalar<double>(copy, App::app()->localContext());
    m_memoizedExpressions[numberOfMemoizedExpressions++] = Unit::BuildTimeSplit(value, App::app()->localContext(), Preferences::sharedPreferences()->complexFormat(), Preferences::sharedPreferences()->angleUnit());
  }
  // 1.d. Simplify and tune prefix of all computed expressions
  size_t currentExpressionIndex = 0;
  while (currentExpressionIndex < numberOfMemoizedExpressions) {
    assert(!m_memoizedExpressions[currentExpressionIndex].isUninitialized());
    if (requireSimplification) {
      Shared::PoincareHelpers::Simplify(&m_memoizedExpressions[currentExpressionIndex], App::app()->localContext(), ExpressionNode::ReductionTarget::User);
    }
    if (canChangeUnitPrefix) {
      Expression newUnits;
      m_memoizedExpressions[currentExpressionIndex] = m_memoizedExpressions[currentExpressionIndex].removeUnit(&newUnits);
      double value = Shared::PoincareHelpers::ApproximateToScalar<double>(m_memoizedExpressions[currentExpressionIndex], App::app()->localContext());
      Unit::ChooseBestPrefixForValue(&newUnits, &value, reductionContext);
      m_memoizedExpressions[currentExpressionIndex] = Multiplication::Builder(Number::FloatNumber(value), newUnits);
    }
    currentExpressionIndex++;
  }

  // 2. IS units only
  assert(numberOfMemoizedExpressions < k_maxNumberOfCells - 1);
  m_memoizedExpressions[numberOfMemoizedExpressions] = m_expression.clone();
  Shared::PoincareHelpers::Simplify(&m_memoizedExpressions[numberOfMemoizedExpressions], App::app()->localContext(), ExpressionNode::ReductionTarget::User, Poincare::ExpressionNode::SymbolicComputation::ReplaceAllDefinedSymbolsWithDefinition, Poincare::ExpressionNode::UnitConversion::InternationalSystem);
  numberOfMemoizedExpressions++;

  // 3. Get rid of duplicates
  currentExpressionIndex = 1;
  while (currentExpressionIndex < numberOfMemoizedExpressions) {
    for (size_t i = 0; i < currentExpressionIndex; i++) {
      assert(!m_memoizedExpressions[i].isUninitialized());
      if (m_memoizedExpressions[i].isIdenticalTo(m_memoizedExpressions[currentExpressionIndex])) {
        numberOfMemoizedExpressions--;
        // Shift next expressions
        for (size_t j = currentExpressionIndex; j < numberOfMemoizedExpressions; j++) {
          m_memoizedExpressions[j] = m_memoizedExpressions[j+1];
        }
        // Remove last expression
        m_memoizedExpressions[numberOfMemoizedExpressions] = Expression();
        // The current expression has been discarded, no need to increment thre current index
        continue;
      }
    }
    // The current expression is not a duplicate, check next expression
    currentExpressionIndex++;
  }
}

int UnitListController::numberOfRows() const {
  int nbOfRows = 0;
  for (size_t i = 0; i < k_maxNumberOfCells; i++) {
    if (!m_memoizedExpressions[i].isUninitialized()) {
      nbOfRows++;
    }
  }
  return nbOfRows;
}

void UnitListController::computeLayoutAtIndex(int index) {
  assert(!m_memoizedExpressions[index].isUninitialized());
  m_layouts[index] = Shared::PoincareHelpers::CreateLayout(m_memoizedExpressions[index]);
}

I18n::Message UnitListController::messageAtIndex(int index) {
  return (I18n::Message)0;
}

}