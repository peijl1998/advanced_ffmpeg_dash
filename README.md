This is based on official FFmpeg git repo(master branch with commit id `758e2da28939c156b18c11c3993ea068da3ea869`)


I add something new related to DASH as follows :)
- ABR algorithm
  - Provide 3 built-in bitrate switch strategy: AlwaysFirst/SimpleThroughput/BBA0
  - Provide a scalable interface to implement your own alrogithm
- Three format support: WEBM/FMP4/MPEG-TS
- SegmentBase Support
  - For webm, dashdec.c will parse segment information from CuePoints
  - For fmp4, dashdec.c will parse segment information from sidx
- Provide two small tools
  - WebmSplitter for split webm file into segments.
  - TsLiveUpdate for simulate mpeg-ts based live (also need MP4Box to convert m3u8 into mpd first)

For more information, please contact me with [this email](peijl.bupt@gmail.com)
