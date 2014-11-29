namespace cpp core

typedef i64 zeonid_t
typedef i64 timestamp_t

typedef i32 nodeid_t

struct NodeId {
  1: nodeid_t nid,
  2: i32 ipv4,
  3: i16 port,
}

struct Point {
  1: i64 xCord = 0,
  2: i64 yCord = 0,
}

struct Version {
  // monotonically increasing value 
  1: i64 counter = -1,
  // timestamp decided at origin of operation
  2: timestamp_t timestamp = 0,
}

struct Data {
  // unique id for Data
  1: zeonid_t id = 0,
  // last-updated location of id
  2: Point point,
  // latest version recoreded for id
  3: Version version,
  // payload for id
  4: string value = "",
}

struct Rectangle {
  1: Point bottomLeft,
  2: Point topRight,
}

struct Region {
  1: list<Rectangle> rectangles,
}

enum ErrorCode {
  FAILED_TO_LOCK = 1,
  STORED = 2,
  FOUND = 3,
  NOT_FOUND = 4,
  FOUND_EMPTY = 5,
  DELETED = 6,
  SERVER_ERROR = 7,
  SERVER_NOT_READY = 8,
  SERVER_REDIRECT = 9,  
}

exception ZeonException {
  1: i32 what,
  2: string why,
  3: optional NodeId node,
}

service PointStore {

  // Heartbeat message
  void ping(),

  // Get last updated data for id, if valuePresent is true then fetch Value
  Data getData (1: zeonid_t id, 2: bool valuePresent=0) 
    throws (1: ZeonException re),

  // Set data as latest value, Version.counter will be incremented by server,
  // If valuePresent is true then set Value in Data
  void setData (1: Data data, 2: bool valuePresent=0) 
    throws (1: ZeonException rc),

  // Create latest data for id with these parameters. Version.counter will be determined by server
  void createData (1: zeonid_t id, 2: Point point, 3: i64 timestamp, 4: string value) 
    throws (1:ZeonException re),

  // Get K Data values nearest to id
  list<Data> getNearestKById (1: zeonid_t id) 
    throws (1:ZeonException re),

  // Get K Data values nearest to point
  list<Data> getNearestKByPoint (1: Point point) 
    throws (1:ZeonException re),

  // Get all Data values inside region
  list<Data> getPointsInRegion (1: Region region) 
    throws (1:ZeonException re),

  // Delete the Data corresponding to id
  void removeData (1: zeonid_t id) 
    throws (1:ZeonException re),
}
