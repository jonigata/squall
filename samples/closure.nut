print("==== script loaded\n");

function bar() {
  print("==== bar called\n");
  foo(@(s)s.len(), "hello world");
  return 7;
}
