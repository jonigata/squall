function zot(foo) {
  print(foo.baz); print("\n");
  print(foo.zot); print("\n");
  foo.baz = 49;
  print(foo.baz); print("\n");
  // foo.zot = 34; // error
  // print(foo.zot); print("\n");
}
