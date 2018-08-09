#include <poincare/addition.h>
//#include <poincare/multiplication.h>
#include <poincare/subtraction.h>
//#include <poincare/power.h>
#include <poincare/opposite.h>
#include <poincare/undefined.h>
//#include <poincare/matrix.h>
#include <assert.h>

namespace Poincare {

static AdditionNode * FailedAllocationStaticNode() {
  static AllocationFailureExpressionNode<AdditionNode> failure;
  return &failure;
}

int AdditionNode::polynomialDegree(char symbolName) const {
  int degree = 0;
  for (int i = 0; i < numberOfChildren(); i++) {
    int d = childAtIndex(i)->polynomialDegree(symbolName);
    if (d < 0) {
      return -1;
    }
    degree = d > degree ? d : degree;
  }
  return degree;
}

int AdditionNode::getPolynomialCoefficients(char symbolName, Expression coefficients[]) const {
  int deg = polynomialDegree(symbolName);
  if (deg < 0 || deg > Expression::k_maxPolynomialDegree) {
    return -1;
  }
  for (int k = 0; k < deg+1; k++) {
    coefficients[k] = Addition();
  }
  Expression intermediateCoefficients[Expression::k_maxNumberOfPolynomialCoefficients];
  for (int i = 0; i < numberOfChildren(); i++) {
    int d = childAtIndex(i)->getPolynomialCoefficients(symbolName, intermediateCoefficients);
    assert(d < Expression::k_maxNumberOfPolynomialCoefficients);
    for (int j = 0; j < d+1; j++) {
      static_cast<Addition *>(&coefficients[j])->addChildAtIndexInPlace(intermediateCoefficients[j], coefficients[j].numberOfChildren(), coefficients[j].numberOfChildren());
    }
  }
  return deg;
}

// Private

// Layout
bool AdditionNode::needsParenthesesWithParent(const SerializationHelperInterface * parentNode) const {
  Type types[] = {Type::Subtraction, Type::Opposite, Type::Multiplication, Type::Division, Type::Power, Type::Factorial};
  return static_cast<const ExpressionNode *>(parentNode)->isOfType(types, 6);
}

LayoutRef AdditionNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Infix(Addition(this), floatDisplayMode, numberOfSignificantDigits, name());
}

// Simplication

Expression AdditionNode::shallowReduce(Context& context, Preferences::AngleUnit angleUnit) const {
  return Addition(this).shallowReduce(context, angleUnit);
}

Expression AdditionNode::shallowBeautify(Context & context, Preferences::AngleUnit angleUnit) const {
  return Addition(this).shallowBeautify(context, angleUnit);
}

Expression AdditionNode::factorizeOnCommonDenominator(Context & context, Preferences::AngleUnit angleUnit) const {
  //TODO
  return Addition();
#if 0
  // We want to turn (a/b+c/d+e/b) into (a*d+b*c+e*d)/(b*d)

  // Step 1: We want to compute the common denominator, b*d
  Multiplication * commonDenominator = new Multiplication();
  for (int i = 0; i < numberOfChildren(); i++) {
    Expression * denominator = childAtIndex(i)->denominator(context, angleUnit);
    if (denominator) {
      // Make commonDenominator = LeastCommonMultiple(commonDenominator, denominator);
      commonDenominator->addMissingFactors(denominator, context, angleUnit);
      delete denominator;
    }
  }
  if (commonDenominator->numberOfChildren() == 0) {
    delete commonDenominator;
    // If commonDenominator is empty this means that no child was a fraction.
    return this;
  }

  // Step 2: Create the numerator. We start with this being a/b+c/d+e/b and we
  // want to create numerator = a/b*b*d + c/d*b*d + e/b*b*d
  AdditionNode * numerator = new AdditionNode();
  for (int i=0; i < numberOfChildren(); i++) {
    Multiplication * m = new Multiplication(childAtIndex(i), commonDenominator, true);
    numerator->addOperand(m);
    m->privateShallowReduce(context, angleUnit, true, false);
  }
  // Step 3: Add the denominator
  Power * inverseDenominator = new Power(commonDenominator, new Rational(-1), false);
  Multiplication * result = new Multiplication(numerator, inverseDenominator, false);

  // Step 4: Simplify the numerator to a*d + c*b + e*d
  numerator->shallowReduce(context, angleUnit);

  // Step 5: Simplify the denominator (in case it's a rational number)
  commonDenominator->deepReduce(context, angleUnit);
  inverseDenominator->shallowReduce(context, angleUnit);

  /* Step 6: We simplify the resulting multiplication forbidding any
   * distribution of multiplication on additions (to avoid an infinite loop). */
  return static_cast<Multiplication *>(replaceWith(result, true))->privateShallowReduce(context, angleUnit, false, true);
#endif
}

void AdditionNode::factorizeOperands(Expression e1, Expression e2, Context & context, Preferences::AngleUnit angleUnit) {
  // TODO
#if 0
  /* This function factorizes two children which only differ by a rational
   * factor. For example, if this is AdditionNode(2*pi, 3*pi), then 2*pi and 3*pi
   * could be merged, and this turned into AdditionNode(5*pi). */
  assert(e1->parent() == this && e2->parent() == this);

  // Step 1: Find the new rational factor
  Rational * r = new Rational(Rational::AdditionNode(RationalFactor(e1), RationalFactor(e2)));

  // Step 2: Get rid of one of the children
  removeOperand(e2, true);

  // Step 3: Use the new rational factor. Create a multiplication if needed
  Multiplication * m = nullptr;
  if (e1->type() == Type::Multiplication) {
    m = static_cast<Multiplication *>(e1);
  } else {
    m = new Multiplication();
    e1->replaceWith(m, false);
    m->addOperand(e1);
  }
  if (m->childAtIndex(0)->type() == Type::Rational) {
    m->replaceOperand(m->childAtIndex(0), r, true);
  } else {
    m->addOperand(r);
  }

  // Step 4: Reduce the multiplication (in case the new rational factor is zero)
  m->shallowReduce(context, angleUnit);
#endif
}

const Rational AdditionNode::RationalFactor(Expression e) {
  if (e.type() == Type::Multiplication && e.childAtIndex(0).type() == Type::Rational) {
    Rational result = Rational(static_cast<RationalNode *>(e.childAtIndex(0).node()));
    return result;
  }
  return Rational(1);
}

static inline int NumberOfNonRationalFactors(const Expression e) {
  // TODO should return a copy?
  if (e.type() != ExpressionNode::Type::Multiplication) {
    return 1; // Or (e->type() != Type::Rational);
  }
  int result = e.numberOfChildren();
  if (e.childAtIndex(0).type() == ExpressionNode::Type::Rational) {
    result--;
  }
  return result;
}

static inline const Expression FirstNonRationalFactor(const Expression e) {
  // TODO should return a copy?
  if (e.type() != ExpressionNode::Type::Multiplication) {
    return e;
  }
  if (e.childAtIndex(0).type() == ExpressionNode::Type::Rational) {
    return e.numberOfChildren() > 1 ? e.childAtIndex(1) : Expression();
  }
  return e.childAtIndex(0);
}

bool AdditionNode::TermsHaveIdenticalNonRationalFactors(const Expression e1, const Expression e2) {
  return false;
//TODO
#if 0
  /* Given two expressions, say wether they only differ by a rational factor.
   * For example, 2*pi and pi do, 2*pi and 2*ln(2) don't. */

  int numberOfNonRationalFactorsInE1 = NumberOfNonRationalFactors(e1);
  int numberOfNonRationalFactorsInE2 = NumberOfNonRationalFactors(e2);

  if (numberOfNonRationalFactorsInE1 != numberOfNonRationalFactorsInE2) {
    return false;
  }

  int numberOfNonRationalFactors = numberOfNonRationalFactorsInE1;
  if (numberOfNonRationalFactors == 1) {
    return FirstNonRationalFactor(e1).isIdenticalTo(FirstNonRationalFactor(e2));
  } else {
    assert(numberOfNonRationalFactors > 1);
    return Multiplication::HaveSameNonRationalFactors(e1, e2);
  }
#endif
}

Expression Addition::shallowBeautify(Context & context, Preferences::AngleUnit angleUnit) const {
  Expression result = *this;
  return result;
  //TODO
#if 0
  /* Beautifying AdditionNode essentially consists in adding Subtractions if needed.
   * In practice, we want to turn "a+(-1)*b" into "a-b". Or, more precisely, any
   * "a+(-r)*b" into "a-r*b" where r is a positive Rational.
   * Note: the process will slightly differ if the negative product occurs on
   * the first term: we want to turn "AdditionNode(Multiplication(-1,b))" into
   * "Opposite(b)".
   * Last but not least, special care must be taken when iterating over children
   * since we may remove some during the process. */

  for (int i=0; i<numberOfChildren(); i++) {
    if (childAtIndex(i)->type() != Type::Multiplication || childAtIndex(i)->childAtIndex(0)->type() != Type::Rational || childAtIndex(i)->childAtIndex(0)->sign() != Sign::Negative) {
      // Ignore terms which are not like "(-r)*a"
      continue;
    }

    Multiplication * m = static_cast<Multiplication *>(editableOperand(i));

    if (static_cast<const Rational *>(m->childAtIndex(0))->isMinusOne()) {
      m->removeOperand(m->childAtIndex(0), true);
    } else {
      m->editableOperand(0)->setSign(Sign::Positive, context, angleUnit);
    }
    Expression * subtractant = m->squashUnaryHierarchy();

    if (i == 0) {
      Opposite * o = new Opposite(subtractant, true);
      replaceOperand(subtractant, o, true);
    } else {
      const Expression * op1 = childAtIndex(i-1);
      removeOperand(op1, false);
      Subtraction * s = new Subtraction(op1, subtractant, false);
      replaceOperand(subtractant, s, false);
      /* CAUTION: we removed a child. So we need to decrement i to make sure
       * the next iteration is actually on the next child. */
      i--;
    }
  }

  return squashUnaryHierarchy();
#endif
}

Expression Addition::shallowReduce(Context& context, Preferences::AngleUnit angleUnit) const {
  // TODO
  return Expression::shallowReduce(context, angleUnit);
#if 0
  Expression * e = Expression::shallowReduce(context, angleUnit);
  if (e != this) {
    return e;
  }
  /* Step 1: AdditionNode is associative, so let's start by merging children which
   * also are additions themselves. */
  int i = 0;
  int initialNumberOfOperands = numberOfChildren();
  while (i < initialNumberOfOperands) {
    Expression * o = editableOperand(i);
    if (o->type() == Type::AdditionNode) {
      mergeOperands(static_cast<AdditionNode *>(o));
      continue;
    }
    i++;
  }

  // Step 2: Sort the children
  sortOperands(Expression::SimplificationOrder, true);

#if MATRIX_EXACT_REDUCING
  /* Step 2bis: get rid of matrix */
  int n = 1;
  int m = 1;
  /* All children have been simplified so if any child contains a matrix, it
   * is at the root node of the child. Moreover, thanks to the simplification
   * order, all matrix children (if any) are the last children. */
  Expression * lastOperand = editableOperand(numberOfChildren()-1);
  if (lastOperand->type() == Type::Matrix) {
    // Create in-place the matrix of addition M (in place of the last child)
    Matrix * resultMatrix = static_cast<Matrix *>(lastOperand);
    n = resultMatrix->numberOfRows();
    m = resultMatrix->numberOfColumns();
    removeOperand(resultMatrix, false);
    /* Scan (starting at the end) accross the addition children to find any
     * other matrix */
    int i = numberOfChildren()-1;
    while (i >= 0 && childAtIndex(i)->type() == Type::Matrix) {
      Matrix * currentMatrix = static_cast<Matrix *>(editableOperand(i));
      int on = currentMatrix->numberOfRows();
      int om = currentMatrix->numberOfColumns();
      if (on != n || om != m) {
        return replaceWith(new Undefined(), true);
      }
      // Dispatch the current matrix children in the created additions matrix
      for (int j = 0; j < n*m; j++) {
        AdditionNode * a = new AdditionNode();
        Expression * resultMatrixEntryJ = resultMatrix->editableOperand(j);
        resultMatrix->replaceOperand(resultMatrixEntryJ, a, false);
        a->addOperand(currentMatrix->editableOperand(j));
        a->addOperand(resultMatrixEntryJ);
        a->shallowReduce(context, angleUnit);
      }
      currentMatrix->detachOperands();
      removeOperand(currentMatrix, true);
      i--;
    }
    // Distribute the remaining addition on matrix children
    for (int i = 0; i < n*m; i++) {
      AdditionNode * a = static_cast<AdditionNode *>(clone());
      Expression * entryI = resultMatrix->editableOperand(i);
      resultMatrix->replaceOperand(entryI, a, false);
      a->addOperand(entryI);
      a->shallowReduce(context, angleUnit);
    }
    return replaceWith(resultMatrix, true)->shallowReduce(context, angleUnit);
  }
#endif

  /* Step 3: Factorize like terms. Thanks to the simplification order, those are
   * next to each other at this point. */
  i = 0;
  while (i < numberOfChildren()-1) {
    Expression * o1 = editableOperand(i);
    Expression * o2 = editableOperand(i+1);
    if (o1->type() == Type::Rational && o2->type() == Type::Rational) {
      Rational r1 = *static_cast<Rational *>(o1);
      Rational r2 = *static_cast<Rational *>(o2);
      Rational a = Rational::AdditionNode(r1, r2);
      replaceOperand(o1, new Rational(a), true);
      removeOperand(o2, true);
      continue;
    }
    if (TermsHaveIdenticalNonRationalFactors(o1, o2)) {
      factorizeOperands(o1, o2, context, angleUnit);
      continue;
    }
    i++;
  }

  /* Step 4: Let's remove zeroes if there's any. It's important to do this after
   * having factorized because factorization can lead to new zeroes. For example
   * pi+(-1)*pi. We don't remove the last zero if it's the only child left
   * though. */
  i = 0;
  while (i < numberOfChildren()) {
    Expression * o = editableOperand(i);
    if (o->type() == Type::Rational && static_cast<Rational *>(o)->isZero() && numberOfChildren() > 1) {
      removeOperand(o, true);
      continue;
    }
    i++;
  }

  // Step 5: Let's remove the addition altogether if it has a single child
  Expression * result = squashUnaryHierarchy();

  // Step 6: Last but not least, let's put everything under a common denominator
  if (result == this && parent()->type() != Type::AdditionNode) {
    // squashUnaryHierarchy didn't do anything: we're not an unary hierarchy
    result = factorizeOnCommonDenominator(context, angleUnit);
  }

  return result;
#endif
}

template Complex<float> Poincare::AdditionNode::compute<float>(std::complex<float>, std::complex<float>);
template Complex<double> Poincare::AdditionNode::compute<double>(std::complex<double>, std::complex<double>);

template MatrixComplex<float> AdditionNode::computeOnMatrices<float>(const MatrixComplex<float>,const MatrixComplex<float>);
template MatrixComplex<double> AdditionNode::computeOnMatrices<double>(const MatrixComplex<double>,const MatrixComplex<double>);

template MatrixComplex<float> AdditionNode::computeOnComplexAndMatrix<float>(std::complex<float> const, const MatrixComplex<float>);
template MatrixComplex<double> AdditionNode::computeOnComplexAndMatrix<double>(std::complex<double> const, const MatrixComplex<double>);

}
