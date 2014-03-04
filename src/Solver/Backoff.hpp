#ifndef xHeinz_Solver_Backoff_HPP
#define xHeinz_Solver_Backoff_HPP

namespace xHeinz {
namespace solver {

class BackOff
{
public:
  typedef enum
  {
    ConstantWaiting,
    LinearWaiting,
    QuadraticWaiting,
    ExponentialWaiting,
    InfiniteWaiting
  } Function;
  
  // constant waiting
  explicit BackOff(int period)
    : _function(ConstantWaiting)
    , _period(0)
    , _attemptsSinceEvent(0)
    , _attemptsUntilNextEvent(period)
  {
  }
  
  BackOff(Function function)
    : _function(function)
    , _period(0)
    , _attemptsSinceEvent(0)
    , _attemptsUntilNextEvent(1)
  {
    assert(function != ConstantWaiting);
    updateWaitingPeriod();
  }
  
  bool makeAttempt()
  {
    if (++_attemptsSinceEvent >= _attemptsUntilNextEvent)
    {
      updateWaitingPeriod();
      _attemptsSinceEvent = 0;
      return true;
    }
    
    return false;
  }
  
  void updateWaitingPeriod()
  {
    switch (_function)
    {
      case ConstantWaiting:
        // nothing to do
        break;
      case LinearWaiting:
        ++_attemptsUntilNextEvent;
        break;
      case QuadraticWaiting:
        ++_period;
        _attemptsUntilNextEvent = _period * _period;
        break;
      case ExponentialWaiting:
        _attemptsUntilNextEvent *= 2;
        break;
      case InfiniteWaiting:
        _attemptsUntilNextEvent = -1;
        break;
    }
  }
  
private:
  Function _function;
  int _period;
  int _attemptsSinceEvent;
  int _attemptsUntilNextEvent;

};
  
} // namespace xHeinz
} // namespace xHeinz

#endif // xHeinz_Solver_Backoff_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
