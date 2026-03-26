// Functions can be defined in any order
func main() {
    greet();
}

func greet() {
    print("Enter your name: ");
    let name = User_input();
    println("Hello " + name);
}
