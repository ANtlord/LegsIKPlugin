LegsIKPlugin
===

Plugin makes inverse kinematics for two legs.

Usage
---

It means, that you have animation blueprint for character, that needs IK.

1. Open animation graph of target animation blueprint.
2. Add node Legs FABRIK.
3. Type name of socket in left foot's tarsus. The same for right foot.
4. Type name of socket in left foot's toe. The same for right foot.
5. Choose root bone.
6. Choose tarsus bone for left and right foots.
7. Choose axis for moving foot. Just try every axis, but, usually it's Z.

Screenshots
---
![Demo](https://forums.unrealengine.com/attachment.php?attachmentid=44744&d=1435035105)
![Demo](https://forums.unrealengine.com/attachment.php?attachmentid=44745&d=1435035159)
![Demo](https://forums.unrealengine.com/attachment.php?attachmentid=44746&d=1435035176)
![Demo](https://forums.unrealengine.com/attachment.php?attachmentid=44747&d=1435035189)

Issues
---
1. IK for more than 2 legs. Spiders, dragons, horse, wolfs etc.
2. IK for moving on walls or under roof.
3. Toe rotation. It needs more correct behavior. Maybe foot must be further.

License
---
 Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
