namespace cpp core

typedef i64 zeon_id

struct Point {
  1: i64 xCord,
  2: i64 yCord,
}

exception InvalidOperation {
  1: i32 what,
  2: string why
}

service PointStore {

   void ping(),

   Point getValue(1:zeon_id key),

   void setKeyValue(1:zeon_id key, 2:Point point),

   oneway void zip()

}
