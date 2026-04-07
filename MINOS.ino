#include <Arduino.h>

#define OS_HEAP_SIZE 256
#define APP_HEAP_SIZE 128

uint8_t os_heap[OS_HEAP_SIZE];
uint8_t app_heap[APP_HEAP_SIZE];
uint16_t os_ptr = 0;
uint16_t app_ptr = 0;

struct Task {
  void (*func)();
  const char* name;
  uint32_t period;
  uint32_t lastRun;
  uint8_t priority;
  uint32_t maxExecTime;
  bool enabled;
};

// Глобальний вказівник на динамічний масив тасків в OS Heap
Task* tasks;
int taskCount = 0;

// Функція виділення пам'яті
void* minos_malloc(size_t size, bool is_system) {
    if (is_system) {
        if (os_ptr + size > OS_HEAP_SIZE) return NULL;
        void* p = &os_heap[os_ptr];
        os_ptr += size;
        return p;
    } else {
        if (app_ptr + size > APP_HEAP_SIZE) return NULL;
        void* p = &app_heap[app_ptr];
        app_ptr += size;
        return p;
    }
}

// Додавання таски прямо в OS Heap
bool add_task(void (*f)(), const char* n, uint32_t p, uint8_t prio, uint32_t limit) {
    Task* new_tasks = (Task*)minos_malloc(sizeof(Task), true);
    if (!new_tasks) return false;
    
    // Оскільки ми просто рухаємо вказівник в os_heap, 
    // перша таска задає початок масиву
    if (taskCount == 0) tasks = new_tasks;
    
    tasks[taskCount] = {f, n, p, 0, prio, limit, true};
    taskCount++;
    return true;
}

void task_Monitor() {
    Serial.print("[SYS] OS Heap: "); Serial.print(os_ptr); Serial.println("/256");
}

void task_Serial() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        if (cmd == "ps") {
            for (int i = 0; i < taskCount; i++) {
                Serial.print(i); Serial.print("\t");
                Serial.print(tasks[i].name); Serial.print("\t");
                Serial.println(tasks[i].enabled ? "ON" : "OFF");
            }
        }
    }
}

void setup() {
    Serial.begin(9600);
    
    // Реєструємо таски в системну купу
    add_task(task_Monitor, "Monitor", 5000, 1, 100);
    add_task(task_Serial, "Serial", 100, 2, 100);
    
    Serial.println("MINOS: Memory segments initialized.");
}

void loop() {
    uint32_t now = millis();
    int best = -1;
    uint8_t topPrio = 255;

    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].enabled && (now - tasks[i].lastRun >= tasks[i].period)) {
            if (tasks[i].priority < topPrio) {
                topPrio = tasks[i].priority;
                best = i;
            }
        }
    }

    if (best != -1) {
        uint32_t start = millis();
        tasks[best].func();
        uint32_t end = millis();
        
        if ((end - start) > tasks[best].maxExecTime) {
            Serial.print("OVERLOAD: "); Serial.println(tasks[best].name);
        }
        tasks[best].lastRun = end;
    }
}