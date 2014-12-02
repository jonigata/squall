print("==== script loaded\n");

function foo0() {
  print("==== foo0 called\n");
  return 0;
}

function foo1(x) {
  print("==== foo1 called, " + x + "\n");
  return 1;
}

function baz() {
  print("==== baz called\n");
  return bar(4649);
}

function foo2() {
  foo_a("hello");
}

function foo3() {
  foo_b("world");
}

