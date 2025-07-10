# Air Traffic Control System – Operating Systems Simulation

This project is a simulation of an **Air Traffic Control System**, built as part of a university Operating Systems course. It models a complex, concurrent environment using low-level OS primitives and demonstrates mastery of **process control**, **inter-process communication**, and **synchronization** in a real-time, multi-process system.

---

## ✈️ Overview

The system simulates air traffic across multiple airports by coordinating the following entities:

- **Planes** (implemented as individual processes)
  - Two types: **Passenger planes** and **Cargo planes**, both take structured user input at runtime.
  - Passenger planes create child processes to represent individual passengers and use **pipes** for communication.
  - Cargo planes take input for the number and average weight of cargo items.
- **Airports** (multi-threaded processes)
  - Handle both arrivals and departures concurrently using threads.
  - Each airport has multiple **runways** with different weight capacities.
  - Runways are allocated to planes using a **best-fit** algorithm.
  - **Mutexes/semaphores** are used to synchronize thread access to runways and prevent conflicts.
- **Air Traffic Controller (ATC)**
  - Coordinates all communication through a **single POSIX message queue**.
  - Logs each flight's journey to a central log file.
- **Cleanup Process**
  - Handles graceful system-wide shutdown after all planes have landed.

The simulation includes realistic constraints such as runway availability, boarding/unloading durations, and asynchronous plane journeys. Passenger and cargo data are collected dynamically during runtime, and system coordination is entirely message-driven.


---

## 💡 Key Concepts Demonstrated

- **POSIX-compliant multi-process architecture**
- **Process creation and management (fork, exec, wait)**
- **Pipes for parent-child IPC**
- **POSIX message queues for multi-process messaging**
- **Multithreading with pthreads**
- **Thread synchronization using mutexes/semaphores**
- **Best-fit allocation of shared runways to planes**
- **Real-time simulation of a distributed, concurrent system**


---

## 🔧 Build & Run

The system is implemented in **C** and is fully **POSIX-compliant**. It is designed to be compiled and executed on Linux systems (tested on Ubuntu 22.04) using `gcc` with `-pthread` support for multithreading.

Each component (plane, airport, controller, cleanup) is compiled independently and executed in separate terminal instances.

> This project is not intended for deployment but demonstrates low-level systems programming concepts in an academic simulation.

---

## 📄 Project Documentation

The repository includes the full problem statement and system specification: 
**[OS Assignment PDF](./OS%20Assignment%202%20-%20Air%20Traffic%20Control%20System%20(1).pdf)**

This document outlines the design constraints, entity behaviors, communication rules, and expected synchronization mechanisms.

---

## 📌 Notes

- All processes terminate automatically after completion or when triggered by the cleanup process.

---

