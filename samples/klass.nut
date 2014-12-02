function zot(foo) {
  print("==== zot called\n");
  //print(foo);
  foo.bar();
  foo.bar();
  foo.bar();
  foo.bar();
  foo.bar();
  foo.bar();
  foo.baz("hello");
}

function quux(foo) {
  print("==== quux called\n");
  foo.qux("world");
}

function zot2(foo2) {
  foo2.bar2();
  foo2.bar();
}
