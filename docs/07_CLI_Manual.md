# CLI User Manual

The Command Line Interface (CLI) allows direct interaction with the Deadlock Detection module. It is the primary way to test the system without the Python GUI.

## Starting the CLI
Run the executable from a terminal:
```powershell
.\bin\deadlock.exe
```

## Main Menu
Upon start, you will see the main menu:
```text
--- Menu ---

Process:   1.Add      2.Remove     3.List
Resource:  4.Add      5.Remove     6.List
Edges:     7.Request  8.Allocate   9.Release
Deadlock:  10.Detect  11.Recover   12.ShowRAG
Demo:      13.Simple  14.Circular  15.Philosophers
Other:     16.API     17.Reset     0.Exit
```

## Common Workflows

### 1. Manual Deadlock Creation
1.  **Add Processes**: Use option `1` to create Process A and Process B.
2.  **Add Resources**: Use option `4` to create Resource R1 and Resource R2.
3.  **Allocate**: Use option `8` to assign R1 to Process A and R2 to Process B.
4.  **Request**: Use option `7` to make Process A request R2, and Process B request R1.
5.  **Detect**: Use option `10`. The system should report "DEADLOCK DETECTED" and list the cycle.

### 2. Running a Demo
1.  Select standard demos like **Simple Deadlock (13)** or **Philosophers (15)**.
2.  The system will automatically populate the RAG.
3.  It will then run detection and print the results.

### 3. Recovery
1.  After detecting a deadlock, select **Recover (11)**.
2.  Choose a strategy from the sub-menu (e.g., "1. Terminate All").
3.  The system will modify the RAG and re-run detection to verify the fix.

## Understanding Output
### RAG Dump (`12. ShowRAG`)
```text
Processes:
  P0: Process A (Priority: 10, State: WAITING)
  P1: Process B (Priority: 10, State: WAITING)
Resources:
  R0: Resource 1 (Instances: 1, Available: 0)
  R1: Resource 2 (Instances: 1, Available: 0)
Edges:
  P0 -> R1 (Request)
  P1 -> R0 (Request)
  R0 -> P0 (Assignment)
  R1 -> P1 (Assignment)
```
This view shows the exact state of the graph. "Assignment" means holding, "Request" means waiting.
