/*
Navicat MySQL Data Transfer

Source Server         : MySQL
Source Server Version : 80017
Source Host           : localhost:3306
Source Database       : chatroom

Target Server Type    : MYSQL
Target Server Version : 80017
File Encoding         : 65001

Date: 2019-09-19 15:29:19
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for chatroom_friend
-- ----------------------------
DROP TABLE IF EXISTS `chatroom_friend`;
CREATE TABLE `chatroom_friend` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_1` int(255) NOT NULL,
  `user_2` int(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `f1` (`user_1`),
  KEY `f2` (`user_2`),
  CONSTRAINT `f1` FOREIGN KEY (`user_1`) REFERENCES `chatroom_user` (`id`),
  CONSTRAINT `f2` FOREIGN KEY (`user_2`) REFERENCES `chatroom_user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- ----------------------------
-- Records of chatroom_friend
-- ----------------------------
INSERT INTO `chatroom_friend` VALUES ('1', '18', '20');
INSERT INTO `chatroom_friend` VALUES ('2', '20', '18');
INSERT INTO `chatroom_friend` VALUES ('7', '18', '22');
INSERT INTO `chatroom_friend` VALUES ('8', '22', '18');
INSERT INTO `chatroom_friend` VALUES ('11', '24', '22');
INSERT INTO `chatroom_friend` VALUES ('12', '22', '24');

-- ----------------------------
-- Table structure for chatroom_room
-- ----------------------------
DROP TABLE IF EXISTS `chatroom_room`;
CREATE TABLE `chatroom_room` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `u1` (`account`)
) ENGINE=InnoDB AUTO_INCREMENT=30 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- ----------------------------
-- Records of chatroom_room
-- ----------------------------
INSERT INTO `chatroom_room` VALUES ('29', '123123');
INSERT INTO `chatroom_room` VALUES ('25', 'q1');
INSERT INTO `chatroom_room` VALUES ('26', 'q2');
INSERT INTO `chatroom_room` VALUES ('27', 'q3');
INSERT INTO `chatroom_room` VALUES ('28', 'test');

-- ----------------------------
-- Table structure for chatroom_roommember
-- ----------------------------
DROP TABLE IF EXISTS `chatroom_roommember`;
CREATE TABLE `chatroom_roommember` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_user` int(11) NOT NULL,
  `id_room` int(11) NOT NULL,
  `user_status` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `f` (`id_room`),
  CONSTRAINT `f` FOREIGN KEY (`id_room`) REFERENCES `chatroom_room` (`id`) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- ----------------------------
-- Records of chatroom_roommember
-- ----------------------------
INSERT INTO `chatroom_roommember` VALUES ('23', '22', '25', '0');
INSERT INTO `chatroom_roommember` VALUES ('24', '22', '26', '0');
INSERT INTO `chatroom_roommember` VALUES ('25', '22', '27', '0');
INSERT INTO `chatroom_roommember` VALUES ('26', '18', '25', '2');
INSERT INTO `chatroom_roommember` VALUES ('27', '20', '25', '2');
INSERT INTO `chatroom_roommember` VALUES ('28', '20', '26', '2');
INSERT INTO `chatroom_roommember` VALUES ('29', '20', '27', '2');
INSERT INTO `chatroom_roommember` VALUES ('30', '22', '28', '0');
INSERT INTO `chatroom_roommember` VALUES ('31', '18', '28', '2');
INSERT INTO `chatroom_roommember` VALUES ('32', '24', '29', '0');
INSERT INTO `chatroom_roommember` VALUES ('33', '22', '29', '2');

-- ----------------------------
-- Table structure for chatroom_user
-- ----------------------------
DROP TABLE IF EXISTS `chatroom_user`;
CREATE TABLE `chatroom_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `passwd` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `u1` (`account`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- ----------------------------
-- Records of chatroom_user
-- ----------------------------
INSERT INTO `chatroom_user` VALUES ('3', '3', '550a141f12de6341fba65b0ad0433500');
INSERT INTO `chatroom_user` VALUES ('16', '86886', '666');
INSERT INTO `chatroom_user` VALUES ('17', '9990', '213');
INSERT INTO `chatroom_user` VALUES ('18', '222', '222');
INSERT INTO `chatroom_user` VALUES ('20', '333', '333');
INSERT INTO `chatroom_user` VALUES ('22', '111', '111');
INSERT INTO `chatroom_user` VALUES ('23', '2', '2');
INSERT INTO `chatroom_user` VALUES ('24', 'test', 'test');
