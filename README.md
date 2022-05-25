# Lexer and Parser
## Sample Source Input
```
// This is a comment
fibonacci(x: int) {
    x : int = fibonacci(x-2) + fibonacci(x+3/2);

    if x < 2 {
        return x;   // This is another comment
    } else {
        return fibonacci(x-1) + fibonacci(x-2);
        x = x + 43 / (2 + 3);
    }
}
```
## Abstract Syntax Tree Visualized Using Graphviz
<p align="center"><img src="ast_output.svg"></p>

## BNF Description of the Grammar
<p align="center"><img src="bnf.jpg"></p>