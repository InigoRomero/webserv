import requests
from requests.auth import HTTPBasicAuth

def printResponse(r):
	print("==STATUSCODE==")
	print(r.status_code)
	print("==HEADERS==")
	print(r.headers)
	print("==BODY==")
	print(r.text)
	print("******************\n")

#print("PUT on 80/put_test")
#r = requests.put('http://localhost:80/put_test')
#printResponse(r)

print("GET on 8080/test/index.html with preferred language EN and preferred encoding utf8")
headers = {'Accept-Language': 'fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5', 'Accept-Charset': 'iso-8859-5, unicode-1-1;q=0.8'}
r = requests.get('http://localhost:80/index.html', headers=headers)
printResponse(r)