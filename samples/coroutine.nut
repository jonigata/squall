print("==== script loaded\n");

function foo() {
  print("==== foo called\n");
  local n = 4649;
  n = suspend(n);
  print("==== foo resumed\n");
  n = suspend(n);
  print("==== foo resumed\n");
  n = suspend(n);
  print("==== foo resumed\n");
  return n;
}

function bar(x, y, z, w) {
  print("==== bar called\n");
  //  return 1;
}

function baz() {
  print("==== baz called\n");
  local n = 7777;
  suspend(n);
  print("==== baz resumed\n");
  suspend(n+1);
  print("==== baz resumed\n");
  suspend(n+2);
  print("==== baz resumed\n");
  return 7;  
}

function zot() {
  print("==== zot called\n");
  suspend();
  print("==== zot resumed\n");
  suspend();
  print("==== zot resumed\n");
  suspend();
  print("==== zot resumed\n");
  return 9;  
}
