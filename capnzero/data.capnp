@0xfc59fe4ba4a77f6b;

struct Content {
    name @0 :Text;
    buffer @1 :Data;
}

struct TheData {
    contents @0 :List(Content);
}
