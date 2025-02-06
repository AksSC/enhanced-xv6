# xv6 Enhancements

This repository contains modifications to the **xv6-riscv** operating system, introducing new system calls, scheduling algorithms, and a **Copy-On-Write (COW) fork** mechanism.

## Features Implemented

### 1. System Calls

#### 🔹 `getSysCount()`
- Tracks how many times a specific system call is invoked by a process and its children.
- The syscall is selected using a **bitmask** in the `syscount` user program.
- **Example Usage**:
  ```sh
  $ syscount 32768 grep hello README.md
  PID 6 called open 1 times.
  ```
  Here, `32768 = 1 << 15`, which corresponds to the `open` syscall.

#### 🔹 `sigalarm(interval, handler)`
- Triggers an alarm **every `n` ticks** of CPU time, invoking the specified `handler` function.
- Used to implement user-level signal handling.

#### 🔹 `sigreturn()`
- Resets the process state after the alarm handler completes.

---

### 2. Scheduling Algorithms ⚡

The default **Round Robin (RR)** scheduler in xv6 has been extended to support:
1. **Lottery-Based Scheduling (LBS)**
2. **Multi-Level Feedback Queue (MLFQ)**

#### 🔹 **Lottery-Based Scheduling (LBS)**
- Each process is assigned a **number of tickets** using:
  ```c
  settickets(int number);
  ```
- A **lottery system** determines which process runs, favoring those with more tickets.
- If two processes have the same ticket count, the **earlier-arriving process** wins.
- Ensures **fair scheduling** while preventing latecomers from overtaking older processes unfairly.

#### 🔹 **Multi-Level Feedback Queue (MLFQ)**
- Implements **4 priority queues**:
  - **Queue 0:** Highest priority (**1 tick** time slice)
  - **Queue 1:** **4 ticks** time slice
  - **Queue 2:** **8 ticks** time slice
  - **Queue 3:** Lowest priority (**16 ticks** time slice)

- **Rules:**
  - Processes start in **Queue 0**.
  - If they use up their time slice, they move **down** a queue.
  - If they **voluntarily yield (I/O-bound)**, they remain in the same queue.
  - After **48 ticks**, all processes get a **priority boost** back to **Queue 0** to prevent starvation.

#### **Performance Comparison (1 CPU)**

| Scheduler | Avg. Running Time | Avg. Waiting Time |
|-----------|-----------------|-----------------|
| **RR** | 23 ticks | 97 ticks |
| **LBS** | 20 ticks | 105 ticks |
| **MLFQ** | 28 ticks | 100 ticks |

- **RR**: Best waiting time.
- **LBS**: Fast execution for CPU-heavy processes but worst waiting time.
- **MLFQ**: Balanced scheduling.

---

### 3. Copy-On-Write (COW) Fork 

- Instead of duplicating all memory pages **immediately**, **COW fork** makes pages **read-only** and **shares them** between parent and child.
- If a process writes to a shared page, a **page fault** occurs, and only then is a **new copy** created.

#### **Benefits:**
- **Reduces memory usage** drastically.
- **Faster process creation** compared to traditional `fork()`.
- **More efficient for multi-process workloads**.

#### **Page Fault Analysis**

| Scenario | Page Faults Observed |
|----------|--------------------|
| Read-Only Operations | 1 |
| Sparse Page Writes | 4 |
| Full Page Writes | 9 |
| Multiple Children Writing | 6 (parent), 4 (each child) |

- **Read-only operations** do not trigger extra page faults.
- **Writes trigger faults only when necessary**, making COW **highly efficient**.

---

## Building & Running xv6

1. Clone the repository:
   ```sh
   git clone https://github.com/AksSC/enhanced-xv6
   cd enhanced-xv6
   ```

2. **Compile with different schedulers**:
   ```sh
   cd src
   make clean; make qemu SCHEDULER=RR    # Default Round Robin
   make clean; make qemu SCHEDULER=LBS   # Lottery-Based Scheduling
   make clean; make qemu SCHEDULER=MLFQ CPUS=1  # Multi-Level Feedback Queue
   ```

3. **Running Tests**:
   ```sh
   make qemu
   schedulertest    # Test scheduling policies
   alarmtest        # Test alarm
   cowtest          # Test Copy-On-Write fork
   ```

---

## Report & Analysis

A detailed **report on implementation and performance analysis** is included in the `docs/` folder.

---

## Credits

This project was implemented by **Aks Kanodia** and **Agyeya Negi** as part of the **Operating Systems and Networks** course.

