#include "ResourceManager.h"
#include <algorithm>
#include <iostream>

void ResourceManager::init(std::string executablePath)
{
    std::replace(executablePath.begin(), executablePath.end(), '\\', '/');
    size_t lastSlashIndex = executablePath.find_last_of('/');
    if (lastSlashIndex != std::string::npos)
        m_assetPath = executablePath.substr(0, lastSlashIndex + 1);
    m_assetPath += + "assets/";
}

const sf::Font* ResourceManager::getOrLoadFont(const std::string& filename)
{
    auto it = m_loadedFonts.find(filename);
    if (it != m_loadedFonts.end())
        return &it->second;

    auto res = m_loadedFonts.emplace(filename, sf::Font());
    if (!res.second)
        return nullptr;
    
    sf::Font* pFont = &res.first->second;
    if (!pFont->openFromFile(getAssetPath(filename)))
        return nullptr;
    return pFont; 
}

const sf::Texture* ResourceManager::getOrLoadTexture(const std::string& filename)
{
    auto it = m_loadedTextures.find(filename);
    if (it != m_loadedTextures.end())
        return &it->second;

    auto res = m_loadedTextures.emplace(filename, sf::Texture());
    if (!res.second)
        return nullptr;

    sf::Texture* pTexture = &res.first->second;
    if (!pTexture->loadFromFile(getAssetPath(filename)))
        return nullptr;
    return pTexture;
}

const sf::SoundBuffer* ResourceManager::getOrLoadSoundBuffer(const std::string& filename)
{
    auto it = m_loadedSoundBuffers.find(filename);
    if (it != m_loadedSoundBuffers.end())
        return &it->second;

    auto res = m_loadedSoundBuffers.emplace(filename, sf::SoundBuffer());
    if (!res.second)
        return nullptr;

    sf::SoundBuffer* pSoundBuffer = &res.first->second;
    if (!pSoundBuffer->loadFromFile(getAssetPath(filename)))
        return nullptr;
    return pSoundBuffer;
}

std::filesystem::path ResourceManager::getAssetPath(const std::string& filename)
{
    std::filesystem::path path = "assets/" + filename;

    return path;
}


