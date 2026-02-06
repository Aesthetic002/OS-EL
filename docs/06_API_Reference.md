# API Reference

The OS-EL backend exposes a **JSON-based API** over standard input/output (stdin/stdout). This allows the Python GUI (or any other frontend) to control the C backend.

## Protocol
-   **Request**: Single line of JSON sent to `stdin`.
-   **Response**: Single line of JSON received from `stdout`.

### Request Format
```json
{
    "command": "CMD_NAME",
    "param1": "value",
    "param2": 123
}
```

### Response Format
```json
{
    "status": "STATUS_SUCCESS", // or STATUS_ERROR
    "message": "Operation completed",
    "data": { ... } // Optional payload
}
```

## Core Commands

### RAG Management
| Command | Description | Parameters |
| :--- | :--- | :--- |
| `CMD_RAG_INIT` | Initialize/Reset RAG | None |
| `CMD_ADD_PROCESS` | Add a process | `name` (string), `priority` (int) |
| `CMD_ADD_RESOURCE`| Add a resource | `name` (string), `instances` (int) |

### Edges
| Command | Description | Parameters |
| :--- | :--- | :--- |
| `CMD_REQUEST_RESOURCE` | $P \rightarrow R$ | `process_id`, `resource_id` |
| `CMD_ALLOCATE_RESOURCE`| $R \rightarrow P$ | `process_id`, `resource_id` |

### Detection & Recovery
| Command | Description | Response Data |
| :--- | :--- | :--- |
| `CMD_DETECT_DEADLOCK` | Run DFS | `{ "deadlock": true, "cycles": [...] }` |
| `CMD_RECOVER` | Run recovery | `{ "success": true, "actions": [...] }` |
| `CMD_RECOMMEND_STRATEGY`| Get advice | `{ "strategy": "RECOVERY_TERMINATE_ALL" }` |

### Simulation
| Command | Description | Parameters |
| :--- | :--- | :--- |
| `CMD_SIM_LOAD_SCENARIO` | Load preset | `scenario` (int ID) |
| `CMD_SIM_TICK` | Advance time | None |
| `CMD_SIM_GET_STATE` | Get full state | Returns RAG + Events |

## Error Handling
If a command fails, the `status` field will be `STATUS_ERROR` and the `message` field will contain a descriptive error string.
