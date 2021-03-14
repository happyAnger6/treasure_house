# CAS

[gcc atomic builtins](https://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Atomic-Builtins.html)

## CMPXCHG

```C
bool compare_and_swap (int *addr, int oldval, int newval)
{
  if ( *addr != oldval ) {
      return false;
  }
  *addr = newval;
  return true;
}
```
