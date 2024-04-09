create database if not exists music_library;

use music_library;

drop table if exists songs;
drop table if exists albums;
drop table if exists artists;

create table if not exists artists(
    artist_id int unsigned not null unique auto_increment,
    artist_name varchar(256) not null,
    primary key(artist_id)
);

create table if not exists albums(
    album_id int unsigned not null unique auto_increment,
    artist_id int unsigned not null,
    album_year bigint not null default 2024,
    album_name varchar(256) not null,
    primary key(album_id),
    foreign key(artist_id) references artists(artist_id)
);

create table if not exists songs(
    song_id int unsigned not null unique auto_increment,
    artist_id int unsigned not null,
    album_id int unsigned not null,
     position int unsigned not null unique,
    song_title varchar(256) not null,
    plays int unsigned not null default 0,
    foreign key(album_id) references albums(album_id),
    foreign key(artist_id) references artists(artist_id)
);
