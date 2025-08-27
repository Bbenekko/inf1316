#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main() {
int X = 0; // Variable to be written by each process
for (int i = 0; i < 3; i++) {
pid_t pid = fork();
if (pid < 0) {
perror("Fork failed");
exit(1);
} else if (pid == 0) {
// Child process
X = i + 1; // Assign a value specific to this child
printf("Child %d: X = %d\n", i + 1, X);
exit(0); // Child terminates
}
// Parent continues to fork next child
}
// Parent process waits for each child
for (int i = 0; i < 3; i++) {
wait(NULL);
X = 100 + i; // Parent's own value after each child's termination
printf("Parent after child %d: X = %d\n", i + 1, X);
}
// Parent writes its final value of X
printf("Parent final X = %d\n", X);
return 0;
}