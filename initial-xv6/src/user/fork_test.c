#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PAGES 8
#define PAGE_SIZE 4096

void print_scenario(char* name) {
    printf("\n=== Scenario: %s ===\n", name);
}

void print_faults(char* scenario) {
    int faults = get_page_faults();
    printf("%s: %d page faults\n", scenario, faults);
}

// Run read-only test
void test_readonly(char* memory) {
    print_scenario("Read-Only Operations");
    int pid = fork();
    
    if(pid < 0) {
        printf("Fork failed!\n");
        exit(1);
    }
    
    if(pid == 0) {
        // Child process - performs extensive read operations
        printf("Child starting read operations...\n");
        print_faults("Before reads");
        
        volatile char sum = 0;  // Prevent optimization
        for(int i = 0; i < PAGES * PAGE_SIZE; i += 512) {  // Increased step for more extensive reads
            sum += memory[i];
        }
        
        print_faults("After reading all pages");
        printf("Read validation sum: %d\n", sum);
        exit(0);
    } else {
        wait(0);
        printf("Parent after read-only child finished\n");
        print_faults("Parent state");
    }
}

// Test partial writes with varying steps to simulate sparse modification
void test_partial_writes(char* memory) {
    print_scenario("Partial Page Writes with Varying Steps");
    int pid = fork();
    
    if(pid < 0) {
        printf("Fork failed!\n");
        exit(1);
    }
    
    if(pid == 0) {
        printf("Child modifying every third page...\n");
        print_faults("Before modifications");
        
        // Modify every third page to simulate sparse writes
        for(int i = 0; i < PAGES; i += 3) {
            memory[i * PAGE_SIZE] = 'B';
            print_faults("After modifying page");
        }
        
        print_faults("Final child state");
        exit(0);
    } else {
        wait(0);
        printf("Parent after partial-write child finished\n");
        print_faults("Parent state");
    }
}

// Test full writes with unique values for each page
void test_full_writes(char* memory) {
    print_scenario("Full Page Writes with Unique Values");
    int pid = fork();
    
    if(pid < 0) {
        printf("Fork failed!\n");
        exit(1);
    }
    
    if(pid == 0) {
        printf("Child modifying all pages with unique values...\n");
        print_faults("Before modifications");
        
        // Modify each page with a distinct character
        for(int i = 0; i < PAGES; i++) {
            memory[i * PAGE_SIZE] = 'C' + i;
            print_faults("After modifying page");
        }
        
        print_faults("Final child state");
        exit(0);
    } else {
        wait(0);
        printf("Parent after full-write child finished\n");
        print_faults("Parent state");
    }
}

// Test multiple children with staggered memory modifications
void test_multiple_children(char* memory) {
    print_scenario("Multiple Children with Staggered Modifications");
    print_faults("Before forking");
    
    for(int child = 0; child < 3; child++) {
        int pid = fork();
        
        if(pid < 0) {
            printf("Fork failed!\n");
            exit(1);
        }
        
        if(pid == 0) {
            printf("Child %d starting...\n", child);
            print_faults("Initial state");
            
            // Each child writes to staggered pages to reduce overlap
            for(int i = child; i < PAGES; i += 3) {
                memory[i * PAGE_SIZE] = 'X' + child;
                print_faults("After page modification");
            }
            
            print_faults("Final child state");
            exit(0);
        }
    }
    
    // Wait for all children
    for(int i = 0; i < 3; i++) {
        wait(0);
    }
    
    printf("Parent after all children finished\n");
    print_faults("Final parent state");
}

int main(int argc, char *argv[])
{
    printf("\n=== COW Page Fault Analysis ===\n");
    
    // Allocate and initialize memory
    char *memory = malloc(PAGES * PAGE_SIZE);
    if(!memory) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    
    // Initialize memory
    for(int i = 0; i < PAGES * PAGE_SIZE; i++) {
        memory[i] = 'A';
    }
    
    print_faults("Initial state");
    
    // Run different test scenarios
    test_readonly(memory);
    test_partial_writes(memory);
    test_full_writes(memory);
    test_multiple_children(memory);
    
    // Cleanup
    free(memory);
    printf("\n=== Analysis Complete ===\n");
    exit(0);
}
