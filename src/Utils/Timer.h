#pragma once

namespace mcctp::utils {
class Timer {
public:
  Timer() { Reset(); }

  void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }

  float Elapsed() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::high_resolution_clock::now() - m_Start)
               .count() *
           0.001f * 0.001f * 0.001f;
  }

  float ElapsedMillis() { return Elapsed() * 1000.0f; }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

/* Windows sleep in 100ns units */
static BOOLEAN qnanosleep(LONGLONG ns) {
  /* Declarations */
  HANDLE timer;     /* Timer handle */
  LARGE_INTEGER li; /* Time defintion */
  /* Create timer */
  if (!(timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                       TIMER_MODIFY_STATE)))
    return FALSE;
  /* Set timer properties */
  li.QuadPart = -ns;
  if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
    CloseHandle(timer);
    return FALSE;
  }
  /* Start & wait for timer */
  WaitForSingleObject(timer, INFINITE);
  /* Clean resources */
  CloseHandle(timer);
  /* Slept without problems */
  return TRUE;
}

} // namespace mcctp::utils