import socks
import socket
import timeit
import stem
import stem.connection
from stem.control import Controller
import urllib2
import StringIO
import pycurl
import os
import subprocess
import time

SOCKS_PORT = 9050
CONNECTION_TIMEOUT = 30

#MIT VERITAS: 3D765C586CCA8B437B7697EA2CE6A51312530AB1
mit = '3D765C586CCA8B437B7697EA2CE6A51312530AB1'
#Stanford everfailed: 5C8540D8D4EF0B7DA4E5486DAF2ECA42EDFEC9C0
stanford = '5C8540D8D4EF0B7DA4E5486DAF2ECA42EDFEC9C0'
#UPenn UnivOfPAExit: EF27BB320827F8C809CA999E07B9B783C7ACC8F1
upenn = 'EF27BB320827F8C809CA999E07B9B783C7ACC8F1'
#UWashington UWashingtonCSE: AE31E0FBF0D3E1B39F1DD8F55BC070D11AF524B0
uwash = 'AE31E0FBF0D3E1B39F1DD8F55BC070D11AF524B0'

def query(url):
     """
     Uses pycurl to fetch a site using the proxy on the SOCKS_PORT.
     """
     output = StringIO.StringIO()

     query = pycurl.Curl()
     query.setopt(pycurl.URL, url)
     query.setopt(pycurl.PROXY, '192.168.1.166')
     query.setopt(pycurl.PROXYPORT, SOCKS_PORT)
     query.setopt(pycurl.PROXYTYPE, pycurl.PROXYTYPE_SOCKS5_HOSTNAME)
     query.setopt(pycurl.CONNECTTIMEOUT, CONNECTION_TIMEOUT)
     query.setopt(pycurl.WRITEFUNCTION, output.write)

     try:
         query.perform()
         return output.getvalue()
     except pycurl.error as exc:
         raise ValueError("Unable to reach %s (%s)" % (url, exc))


def create_circuit(controller):
    controller.authenticate()

#    path = [mit, stanford, upenn]
    path = [mit, upenn, uwash]
    circuit_id = controller.new_circuit(path=path, await_build=True)

    def attach_stream(stream):
        if stream.status == 'NEW':
            controller.attach_stream(stream.id, circuit_id)
    controller.add_event_listener(attach_stream, stem.control.EventType.STREAM)

    try:
        controller.set_conf('__LeaveStreamsUnattached', '1')

        check_page = query('https://check.torproject.org/')
        if 'Congratulations. This browser is configured to use Tor.' not in check_page:
            raise ValueError("Request didn't have the right content")
    finally:
        controller.remove_event_listener(attach_stream)
        controller.reset_conf('__LeaveStreamsUnattached')

def download_file():
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "192.168.1.166", 9050)
    socket.socket = socks.socksocket
    #u = urllib2.urlopen('http://releases.ubuntu.com/15.04/ubuntu-15.04-desktop-i386.iso')#.read()
    #u = urllib2.urlopen('https://www.google.com')#.read()
    u = urllib2.urlopen('https://raw.githubusercontent.com/mcoughli/xilinx_zedboard_c/master/test_blob')
    #file_name = 'ubuntu-15.04-desktop-i386.iso'
    file_name = 'test_blob'
    f = open(file_name, 'w')
    meta = u.info()
    file_size = int(meta.getheaders("Content-Length")[0])
    print "Downloading: %s Bytes: %s" % (file_name, file_size)
    
    downloaded = 0
    start_time = timeit.default_timer()
    while True:
        buffer = u.read(8192)
        if not buffer:
            break
        downloaded += len(buffer)
        f.write(buffer)
        status = r"%10d  [%3.2f%%]" % (downloaded, downloaded * 100. / file_size)
        status = status + chr(8)*(len(status)+1)
        print status,
    elapsed = timeit.default_timer() - start_time
    print 'Average KB/s: ' + str(downloaded/1024/elapsed)
    return downloaded, elapsed

def upload_file():
    file_name = 'test_blob_5M.bin'
    f = open(file_name, 'r')
    file_contents = f.read()

    send_bytes = len(file_contents)

    print 'Sending ' + str(send_bytes) + ' bytes'

    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "localhost", 9050)
#    socket.socket = socks.socksocket
    s = socks.socksocket()
    s.connect("128.138.202.145", 9000)

    start_time = timeit.default_timer()
    s.write(file_contents)
    elapsed - timeit.default_timer() - start_time
    print 'Average KB/s: ' + str(send_bytes/1024/elapsed)
    return send_bytes, elapsed



                        
if __name__ == "__main__":
    p = subprocess.Popen("/home/michael/tor_out/bin/tor -f /home/michael/torrc", shell=True)
    time.sleep(3)
    controller = Controller.from_port(port=9100)
    create_circuit(controller)
    upload_file()
#    running_bps_sum = 0
 #   for i in range(10):
#        downloaded, elapsed = download_file()
#        running_bps_sum += (downloaded/elapsed)
#    print "Average of 10 downloads: "+str(running_bps_sum/1024/10)+" KB/s"
    
#    os.system("proxychains4 iperf -c 128.138.202.145 -n 5M -p 9000 >> test.log")
    controller.close()
