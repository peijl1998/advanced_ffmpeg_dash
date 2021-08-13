import copy
import argparse
import time
import xml.sax
import xml.sax.handler

class WorksHandler(xml.sax.ContentHandler):
    def __init__(self):
        self.reps = []
    def startElement(self, tag, attr):
        if tag == "Representation":
            self.reps.append({"segs": []})
            self.reps[-1]["bw"] = attr["bandwidth"]
        if tag == "SegmentURL":
            self.reps[-1]["segs"].append(attr["media"])

def parse(path):
    parser = xml.sax.make_parser()
    parser.setFeature(xml.sax.handler.feature_namespaces,0)
    Handler = WorksHandler()
    parser.setContentHandler(Handler)
    parser.parse(path)

    reps = Handler.reps
    return reps[0]

def get_header(begin_time, update_time):
    s = '<?xml version="1.0"?>\n'
    s += '<MPD xmlns="urn:mpeg:dash:schema:mpd:2011"\n'
    s += 'profiles="urn:mpeg:dash:profile:isoff-live:2011"\n'
    s += 'type="dynamic"\n'
    s += 'minimumUpdatePeriod="PT10S"\n'
    s += 'suggestedPresentationDelay="PT6S"\n'
    s += 'availabilityStartTime="2021-08-13T05:57:10.664Z"\n'.format(begin_time[0], begin_time[1])
    s += 'publishTime="2021-08-13T05:57:16.548Z"\n'.format(update_time[0], update_time[1])
    s += 'timeShiftBufferDepth="PT2M0.0S"\n'
    s += 'maxSegmentDuration="PT10.0S"\n'
    s += 'minBufferTime="PT12.0S">\n'
    s += '<Period id="0" start="PT0.0S">\n'
    return s

def get_tailer():
    return '</Period>\n</MPD>'

def get_rep(reps):
    s = ""
    for rep in reps:
        s += '<Representation id="{}" mimeType="{}" bandwidth="{}">\n'.format(rep["id"], rep["type"], rep["bw"])
        
        s += '<SegmentTemplate timescale="1000" media="{}_$Number$.ts" startNumber="{}">'.format(rep["name"], rep["start"])
        s+= '</SegmentTemplate>' 
        s += '</Representation>\n'

    return s

def get_as(i, ct, reps):
    s = '<AdaptationSet id="{}" contentType="{}" startWithSAP="1">\n'.format(i, ct)
    s += get_rep(reps)
    s += '</AdaptationSet>\n'
    return s
def pt(t):
    return [time.strftime("%Y-%m-%d", time.localtime()), time.strftime("%H:%M:%S", time.localtime())]

def generate_mpd(begin_time, update_time, video_reps, audio_reps):
    s = get_header(pt(begin_time), pt(update_time))
    s += get_as(0, "video", video_reps)
    s += get_as(1, "audio", audio_reps)
    s += get_tailer()

    return s


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", "--video", help="video mpd list, e.g. 1.mpd,2.mpd,...")
    parser.add_argument("-a", "--audio", help="audio mpd list, e.g. 1.mpd,2.mpd,...")
    args = parser.parse_args()
    begin_time = time.localtime()
    
    videos = []
    for vp in args.video.split(","):
        videos.append(parse(vp))

    audios = []
    for ap in args.audio.split(","):
        audios.append(parse(ap))

    v_idx = 0
    a_idx = 0
    while True:
        video_reps, audio_reps = [], []
        for i, v in enumerate(videos):
            temp = copy.deepcopy(v)
            temp["name"] = "video/" + "_".join(v["segs"][0].split("_")[:-1])
            # temp["segs"] = v["segs"][v_idx : min(len(v["segs"]), v_idx + 2)]
            # temp["segs"] = ["video/" + x for x in temp["segs"]]
            temp["id"] = i
            temp["start"] = v_idx
            temp["type"] = "video/mp2t"
            video_reps.append(temp)
        v_idx = (v_idx + 2) % len(videos[0]["segs"])
        for i, v in enumerate(audios):
            temp = copy.deepcopy(v)
            temp["name"] = "audio/" + "_".join(v["segs"][0].split("_")[:-1])
            # temp["segs"] = v["segs"][a_idx : min(len(v["segs"]), a_idx + 2)]
            # temp["segs"] = ["audio/" + x for x in temp["segs"]]
            temp["id"] = i
            temp["type"] = "audio/mp2t"
            temp["start"] = a_idx
            audio_reps.append(temp)
        a_idx = (a_idx + 2) % len(audios[0]["segs"])
        
        s = generate_mpd(begin_time, time.localtime(), video_reps, audio_reps)
        with open("manifest.mpd", "w") as f:
            f.write(s)

        time.sleep(18)
