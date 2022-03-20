
extern int printf(const char *__format, ...);

int memo[25];
int fib(int n) {
    if (n <= 1) return memo[n] = n;
    if (memo[n]) return memo[n];
    return memo[n] = fib(n - 1) + fib(n - 2);
}

int main() {
    int x = fib(20);
    for (int i = 0;i <= 20; i++) {
        printf("fib(%d) = %d\n", i, memo[i]);
    }
    return 0;
}