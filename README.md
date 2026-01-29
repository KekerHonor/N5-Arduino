# 🦞 Claw Subsystem: Operation Manual & Technical Guide

### System Overview
The claw operates on an **Automated State Machine**. It does not require manual button presses to open/close. Instead, it uses an ultrasonic distance sensor to decide when to grab, hold, or drop objects based on the arm's height from the ground.

---

## 🏎️ 1. Driver’s Guide (Competition Day)
*How to drive the robot to make the claw work correctly.*

### **STEP 1: The Pickup (Grab)**
* **Action:** Lower the arm so the claw is **within 27 cm** of the object/ground.
* **Wait:** You must hold the arm steady for **1.5 seconds**.
* **Indicator:** The claw will close slowly (Servo moves `0` $\to$ `100`).
* > **Note:** If you move the arm away before 1.5 seconds, the timer resets.

### **STEP 2: Transport (Arming)**
* **Action:** Immediately after the claw closes, **LIFT the arm high (above 40 cm)**.
* **Why:** This tells the code *"I have successfully grabbed the object."* The claw is now **"Armed"** and ready to travel.

### **STEP 3: The Drop**
* **Action:** Drive to the target zone and lower the arm **below 27 cm**.
* **Result:** The claw will open **instantly** (no delay).
* > **Note:** Ensure the drop zone is clear before lowering.

### **STEP 4: Reset**
* **Action:** Lift the arm back up (**above 40 cm**) to clear the mechanism.
* **Result:** The system resets to **"Ready"** mode and can grab the next object.

---

## 🛡️ 2. Safety Features (The "Fumble" Logic)
*What happens if the claw misses?*

If the claw closes but you **do not lift the arm** (height stays below 27 cm):
1.  The system assumes a **"Fumble"** (missed grab).
2.  After **3 seconds**, the claw will automatically **OPEN** and reset.

> **Driver Action:** Do not panic. Just wait 3 seconds, realign, and the grab timer will start again automatically.

---

## 🧠 3. Logic States Explained

The code cycles through 4 logic states. You can visualize the flow below:

```mermaid
graph TD
    A[State 1: Ready to Grab] -->|Dist < 27cm for 1.5s| B[State 2: Lifting / Verify]
    B -->|Dist > 40cm| C[State 3:
