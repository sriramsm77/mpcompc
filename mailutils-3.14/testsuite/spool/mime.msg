From gray@example.net Sat Jul 13 00:50:58 2002
Organization: Mailutils-tests
Received: from example.net (localhost [127.0.0.1])
	by example.net with ESMTP id g6CLowb05126
	for <gray@example.net>; Sat, 13 Jul 2002 00:50:58 +0300
Message-Id: <200207122150.g6CLowb05126@example.net>
To: Foo Bar <foobar@nonexistent.net>
Subject: Nested MIME
MIME-Version: 1.0
Content-Type: multipart/mixed; boundary="----- =_aaaaaaaaaa0"
Content-ID: <5122.1026510654.1@example.net>
Date: Sat, 13 Jul 2002 00:50:58 +0300
From: Sergey Poznyakoff <gray@example.net>

------- =_aaaaaaaaaa0
Content-Type: text/plain; name="msg.21"; charset="us-ascii"
Content-ID: <5122.1026510654.2@example.net>
Content-Description: Father William Part I

`You are old, Father William,' the young man said,
`And your hair has become very white;
And yet you incessantly stand on your head--
Do you think, at your age, it is right?'

`In my youth,' Father William replied to his son,
`I feared it might injure the brain;
But, now that I'm perfectly sure I have none,
Why, I do it again and again.'


------- =_aaaaaaaaaa0
Content-Type: multipart/mixed; boundary="----- =_aaaaaaaaaa1"
Content-ID: <5122.1026510654.3@example.net>

------- =_aaaaaaaaaa1
Content-Type: application/octet-stream; name="msg.22"
Content-ID: <5122.1026510654.4@example.net>
Content-Description: Father William Part II
Content-Transfer-Encoding: base64

YFlvdSBhcmUgb2xkLCcgc2FpZCB0aGUgeW91dGgsIGBhcyBJIG1lbnRpb25lZCBiZWZvcmUsCkFu
ZCBoYXZlIGdyb3duIG1vc3QgdW5jb21tb25seSBmYXQ7CllldCB5b3UgdHVybmVkIGEgYmFjay1z
b21lcnNhdWx0IGluIGF0IHRoZSBkb29yLS0KUHJheSwgd2hhdCBpcyB0aGUgcmVhc29uIG9mIHRo
YXQ/JwoKYEluIG15IHlvdXRoLCcgc2FpZCB0aGUgc2FnZSwgYXMgaGUgc2hvb2sgaGlzIGdyZXkg
bG9ja3MsCmBJIGtlcHQgYWxsIG15IGxpbWJzIHZlcnkgc3VwcGxlCkJ5IHRoZSB1c2Ugb2YgdGhp
cyBvaW50bWVudC0tb25lIHNoaWxsaW5nIHRoZSBib3gtLQpBbGxvdyBtZSB0byBzZWxsIHlvdSBh
IGNvdXBsZT8nCg==

------- =_aaaaaaaaaa1
Content-Type: multipart/mixed; boundary="----- =_aaaaaaaaaa2"
Content-ID: <5122.1026510654.5@example.net>

------- =_aaaaaaaaaa2
Content-Type: application/octet-stream; name="msg.23"
Content-ID: <5122.1026510654.6@example.net>
Content-Description: Father William Part III
Content-Transfer-Encoding: base64

YFlvdSBhcmUgb2xkLCcgc2FpZCB0aGUgeW91dGgsIGBhbmQgeW91ciBqYXdzIGFyZSB0b28gd2Vh
awpGb3IgYW55dGhpbmcgdG91Z2hlciB0aGFuIHN1ZXQ7CllldCB5b3UgZmluaXNoZWQgdGhlIGdv
b3NlLCB3aXRoIHRoZSBib25lcyBhbmQgdGhlIGJlYWstLQpQcmF5IGhvdyBkaWQgeW91IG1hbmFn
ZSB0byBkbyBpdD8nCgpgSW4gbXkgeW91dGgsJyBzYWlkIGhpcyBmYXRoZXIsIGBJIHRvb2sgdG8g
dGhlIGxhdywKQW5kIGFyZ3VlZCBlYWNoIGNhc2Ugd2l0aCBteSB3aWZlOwpBbmQgdGhlIG11c2N1
bGFyIHN0cmVuZ3RoLCB3aGljaCBpdCBnYXZlIHRvIG15IGphdywKSGFzIGxhc3RlZCB0aGUgcmVz
dCBvZiBteSBsaWZlLicK

------- =_aaaaaaaaaa2
Content-Type: application/octet-stream; name="msg.24"
Content-ID: <5122.1026510654.7@example.net>
Content-Description: Father William Part IV
Content-Transfer-Encoding: base64

YFlvdSBhcmUgb2xkLCcgc2FpZCB0aGUgeW91dGgsIGBvbmUgd291bGQgaGFyZGx5IHN1cHBvc2UK
VGhhdCB5b3VyIGV5ZSB3YXMgYXMgc3RlYWR5IGFzIGV2ZXI7CllldCB5b3UgYmFsYW5jZWQgYW4g
ZWVsIG9uIHRoZSBlbmQgb2YgeW91ciBub3NlLS0KV2hhdCBtYWRlIHlvdSBzbyBhd2Z1bGx5IGNs
ZXZlcj8nCgpgSSBoYXZlIGFuc3dlcmVkIHRocmVlIHF1ZXN0aW9ucywgYW5kIHRoYXQgaXMgZW5v
dWdoLCcKU2FpZCBoaXMgZmF0aGVyOyBgZG9uJ3QgZ2l2ZSB5b3Vyc2VsZiBhaXJzIQpEbyB5b3Ug
dGhpbmsgSSBjYW4gbGlzdGVuIGFsbCBkYXkgdG8gc3VjaCBzdHVmZj8KQmUgb2ZmLCBvciBJJ2xs
IGtpY2sgeW91IGRvd24gc3RhaXJzIScK

------- =_aaaaaaaaaa2--

------- =_aaaaaaaaaa1--

------- =_aaaaaaaaaa0--

