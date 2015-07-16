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